from __future__ import division
import os
import time
import csv
import random
import cv2
import scipy.io
from random import randint
from math import floor, sqrt, pi
from glob import glob
import tensorflow as tf
from tensorboard import summary as summary_lib
from tensorboard.plugins.custom_scalar import layout_pb2
import numpy as np
from six.moves import xrange

from prefetch_generator import background
from vgg_face import vgg_face
import imageio
from random import randrange

class RelightNet(object):
    def __init__(self, sess):
        self.sess = sess
        self.batch_size = 5
        self.img_height = 256
        self.img_width = 256
        self.SH_dim = 9
        self.lr = 0.0001
        self.df_dim = 64
        self.build_model()

    def build_model(self):
        self.input_luminance_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='input_luminance')
        self.source_border_weights_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='source_border_weights')
        self.target_border_weights_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='target_border_weights')
        self.gt_ratio_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='gt_ratio')
        self.source_lighting_ph = tf.placeholder(tf.float32, [self.batch_size, self.SH_dim], name='SH_source')
        self.target_lighting_ph = tf.placeholder(tf.float32, [self.batch_size, self.SH_dim], name='SH_target')
        self.epoch_ph = tf.placeholder(tf.float32, [], name='epoch_num')
        self.zero_tensor = tf.constant(0, shape=[self.batch_size, self.img_height, self.img_width, 1], dtype=tf.float32)
        self.reverse_gamma_correction_tensor = tf.constant(2.2, shape=[self.batch_size, self.img_height, self.img_width, 1], dtype=tf.float32)
        self.target_luminance_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='target_luminance')
        self.gan_loss_scale_factor_ph = tf.placeholder(tf.float32, name='gan_scale_factor')

        (self.predicted_ratio, self.predicted_source_lighting, self.identity_features) = self.relighting_UNET(self.input_luminance_ph, self.target_lighting_ph, True, self.epoch_ph)

        self.predicted_ratio = tf.clip_by_value(self.predicted_ratio, clip_value_min=0.0000000001, clip_value_max=10000000000)

        self.positive_weights_source = tf.cast(tf.greater(self.source_border_weights_ph, self.zero_tensor), tf.float32)
        self.positive_weights_target = tf.cast(tf.greater(self.target_border_weights_ph, self.zero_tensor), tf.float32)

        self.source_shadow_border_loss = 3.0*tf.reduce_sum(self.source_border_weights_ph*tf.abs(self.log10(self.predicted_ratio)-self.log10(self.gt_ratio_ph)))/(tf.reduce_sum(self.positive_weights_source)+0.00001)
        self.target_shadow_border_loss = 3.0*tf.reduce_sum(self.target_border_weights_ph*tf.abs(self.log10(self.predicted_ratio)-self.log10(self.gt_ratio_ph)))/(tf.reduce_sum(self.positive_weights_target)+0.00001)
        self.target_ratio_loss = 3.0*tf.reduce_sum(tf.abs(self.log10(self.predicted_ratio)-self.log10(self.gt_ratio_ph)))/(self.img_height*self.img_width*self.batch_size)
        self.source_lighting_loss = tf.reduce_sum(tf.square(self.predicted_source_lighting-self.source_lighting_ph))/self.batch_size
        self.dx_predicted, self.dy_predicted = tf.image.image_gradients(self.log10(self.predicted_ratio))
        self.dx_gt, self.dy_gt = tf.image.image_gradients(self.log10(self.gt_ratio_ph))
        self.image_gradient_loss = (tf.reduce_sum(tf.abs(self.dx_gt-self.dx_predicted))+tf.reduce_sum(tf.abs(self.dy_gt-self.dy_predicted)))/(self.img_height*self.img_width*self.batch_size)

        self.repeated_batch_identity_features = tf.tile(self.identity_features, [self.batch_size, 1, 1, 1])
        self.repeated_batch_identity_features2 = self.tf_repeat(self.identity_features, tf.fill([self.batch_size], self.batch_size))

        self.consistency_loss_identity = tf.reduce_sum(tf.abs(self.repeated_batch_identity_features-self.repeated_batch_identity_features2))/((self.batch_size-1)*self.batch_size*self.img_height/16.0*self.img_width/16.0)
        self.SSIM_loss = (1.0-tf.reduce_mean(tf.image.ssim(self.log10(self.gt_ratio_ph), self.log10(self.predicted_ratio), 2.0)))/2.0

        self.predicted_luminance = self.predicted_ratio*self.input_luminance_ph
        self.predicted_luminance = tf.pow(self.predicted_luminance, self.reverse_gamma_correction_tensor)

        self.real_D_256, self.real_D_logits_256 = self.discriminator_256(self.target_luminance_ph, is_training=True, reuse=False)
        self.fake_D_256, self.fake_D_logits_256 = self.discriminator_256(self.predicted_luminance, is_training=True, reuse=True)
        self.d_loss_real_256 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.real_D_logits_256, labels=tf.ones_like(self.real_D_256)))
        self.d_loss_fake_256 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.fake_D_logits_256, labels=tf.zeros_like(self.fake_D_256)))

        self.predicted_luminance_128 = tf.image.resize_nearest_neighbor(self.predicted_luminance, (128, 128))
        self.target_luminance_128 = tf.image.resize_nearest_neighbor(self.target_luminance_ph, (128, 128))
        self.real_D_128, self.real_D_logits_128 = self.discriminator_128(self.target_luminance_128, is_training=True, reuse=False)
        self.fake_D_128, self.fake_D_logits_128 = self.discriminator_128(self.predicted_luminance_128, is_training=True, reuse=True)
        self.d_loss_real_128 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.real_D_logits_128, labels=tf.ones_like(self.real_D_128)))
        self.d_loss_fake_128 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.fake_D_logits_128, labels=tf.zeros_like(self.fake_D_128)))

        self.g_loss_256 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.fake_D_logits_256, labels=tf.ones_like(self.fake_D_256)))  
        self.g_loss_128 = self.gan_loss_scale_factor_ph*tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=self.fake_D_logits_128, labels=tf.ones_like(self.fake_D_128)))
        self.g_loss = (self.g_loss_256+self.g_loss_128)/2.0
        
        self.total_g_loss = tf.cond(tf.greater(self.epoch_ph, 16), lambda: self.target_ratio_loss+self.source_lighting_loss+self.image_gradient_loss+self.consistency_loss_identity+self.SSIM_loss+self.source_shadow_border_loss+self.target_shadow_border_loss+self.g_loss, lambda: self.target_ratio_loss+self.source_lighting_loss+self.image_gradient_loss+self.SSIM_loss+self.source_shadow_border_loss+self.target_shadow_border_loss+self.g_loss)

        self.total_d_loss_256 = self.d_loss_real_256+self.d_loss_fake_256
        self.total_d_loss_128 = self.d_loss_real_128+self.d_loss_fake_128

    def relighting_UNET(self, input_luminance, target_lighting, is_training, epoch):
        c1_og = tf.nn.relu(self.batch_norm(self.conv2d(input_luminance, 16, kernel=(5,5), strides=(1, 1), use_bias = True, name='c1'), train=is_training, name='bn_c1'))

        c1 = tf.nn.max_pool(c1_og, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='VALID')
        h1_1 = tf.nn.relu(self.batch_norm(self.conv2d(c1, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='h1_1'), train=is_training, name='bn_h1_1'))
        h1_2 = self.batch_norm(self.conv2d(h1_1, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='h1_2'), train=is_training, name='bn_h1_2')
        h1_out_og = tf.nn.relu(c1+h1_2)

        h1_out = tf.nn.max_pool(h1_out_og, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='VALID')
        h2_1 = tf.nn.relu(self.batch_norm(self.conv2d(h1_out, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='h2_1'), train=is_training, name='bn_h2_1'))
        h2_2 = self.batch_norm(self.conv2d(h2_1, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='h2_2'), train=is_training, name='bn_h2_2')
        shortcut_h1_out = self.batch_norm(self.conv2d(h1_out, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_h1_out'), train=is_training, name='bn_sc_h1_out')
        h2_out_og = tf.nn.relu(shortcut_h1_out+h2_2)

        h2_out = tf.nn.max_pool(h2_out_og, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='VALID')
        h3_1 = tf.nn.relu(self.batch_norm(self.conv2d(h2_out, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='h3_1'), train=is_training, name='bn_h3_1'))
        h3_2 = self.batch_norm(self.conv2d(h3_1, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='h3_2'), train=is_training, name='bn_h3_2')
        shortcut_h2_out = self.batch_norm(self.conv2d(h2_out, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_h2_out'), train=is_training, name='bn_sc_h2_out')
        h3_out_og = tf.nn.relu(shortcut_h2_out+h3_2)

        h3_out = tf.nn.max_pool(h3_out_og, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='VALID')
        h4_1 = tf.nn.relu(self.batch_norm(self.conv2d(h3_out, 155, kernel=(3,3), strides=(1,1), use_bias=True, name='h4_1'), train=is_training, name='bn_h4_1'))
        h4_2 = self.batch_norm(self.conv2d(h4_1, 155, kernel=(3,3), strides=(1,1), use_bias=True, name='h4_2'), train=is_training, name='bn_h4_2')
        shortcut_h3_out = self.batch_norm(self.conv2d(h3_out, 155, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_h3_out'), train=is_training, name='bn_sc_h3_out')
        h4_out = tf.nn.relu(shortcut_h3_out+h4_2)

        identity_features = h4_out[:, :, :, 0:128]
        lighting_features = h4_out[:, :, :, 128:155]
        LF_shape = self.get_shape(lighting_features)
        LF_avg_pool = tf.nn.avg_pool(lighting_features, ksize = [1, LF_shape[1], LF_shape[2], 1], strides = [1,1,1,1], padding = 'VALID')
        SL_lin1 = self.linear(LF_avg_pool, 128, scope= 'SL_lin1')
        SL_lin2 = self.linear(SL_lin1, 9, scope='SL_lin2', relu=False)

        TL_lin1 = self.linear(target_lighting, 128, scope='TL_lin1')
        TL_lin2 = self.linear(TL_lin1, 27, scope='TL_lin2')
        TL_lin2 = tf.expand_dims(TL_lin2, 1)
        TL_lin2 = tf.expand_dims(TL_lin2, 1)
        tile_multiples = tf.constant([1,16,16,1], tf.int32)
        TL_lin2 = tf.tile(TL_lin2, tile_multiples)
        
        all_features = tf.concat([identity_features, TL_lin2], axis=-1)
        h5_1 = tf.nn.relu(self.batch_norm(self.deconv2d(all_features, 64, kernel=(3,3), strides=(1, 1), use_bias = True, name='h5_1'), train=is_training, name='bn_h5_1'))
        h5_2 = self.batch_norm(self.deconv2d(h5_1, 64, kernel=(3,3), strides=(1, 1), use_bias = True, name='h5_2'), train=is_training, name='bn_h5_2')
        shortcut_all_features = self.batch_norm(self.deconv2d(all_features, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_all_features'), train=is_training, name='bn_sc_all_features')
        h5_out = tf.nn.relu(shortcut_all_features+h5_2)
        h5_out = tf.image.resize_nearest_neighbor(h5_out, (32, 32))

        skip_s1_1 = tf.nn.relu(self.batch_norm(self.conv2d(h3_out_og, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s1_1'), train=is_training, name='bn_skip_s1_1'))
        skip_s1_2 = self.batch_norm(self.conv2d(skip_s1_1, 64, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s1_2'), train=is_training, name='bn_skip_s1_2')
        skip_s1_out = tf.nn.relu(h3_out_og+skip_s1_2)
        h5_out = tf.cond(tf.greater(epoch, 8), lambda: h5_out+skip_s1_out, lambda: h5_out)

        h6_1 = tf.nn.relu(self.batch_norm(self.deconv2d(h5_out, 32, kernel=(3,3), strides=(1, 1), use_bias = True, name='h6_1'), train=is_training, name='bn_h6_1'))
        h6_2 = self.batch_norm(self.deconv2d(h6_1, 32, kernel=(3,3), strides=(1, 1), use_bias = True, name='h6_2'), train=is_training, name='bn_h6_2')
        shortcut_h5_out = self.batch_norm(self.deconv2d(h5_out, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_h5_out'), train=is_training, name='bn_sc_h5_out')
        h6_out = tf.nn.relu(shortcut_h5_out+h6_2)
        h6_out = tf.image.resize_nearest_neighbor(h6_out, (64, 64))

        skip_s2_1 = tf.nn.relu(self.batch_norm(self.conv2d(h2_out_og, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s2_1'), train=is_training, name='bn_skip_s2_1'))
        skip_s2_2 = self.batch_norm(self.conv2d(skip_s2_1, 32, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s2_2'), train=is_training, name='bn_skip_s2_2')
        skip_s2_out = tf.nn.relu(h2_out_og+skip_s2_2)
        h6_out = tf.cond(tf.greater(epoch, 10), lambda: h6_out+skip_s2_out, lambda: h6_out)

        h7_1 = tf.nn.relu(self.batch_norm(self.deconv2d(h6_out, 16, kernel=(3,3), strides=(1, 1), use_bias = True, name='h7_1'), train=is_training, name='bn_h7_1'))
        h7_2 = self.batch_norm(self.deconv2d(h7_1, 16, kernel=(3,3), strides=(1, 1), use_bias = True, name='h7_2'), train=is_training, name='bn_h7_2')
        shortcut_h6_out = self.batch_norm(self.deconv2d(h6_out, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='sc_h6_out'), train=is_training, name='bn_sc_h6_out')
        h7_out = tf.nn.relu(shortcut_h6_out+h7_2)
        h7_out = tf.image.resize_nearest_neighbor(h7_out, (128, 128))

        skip_s3_1 = tf.nn.relu(self.batch_norm(self.conv2d(h1_out_og, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s3_1'), train=is_training, name='bn_skip_s3_1'))
        skip_s3_2 = self.batch_norm(self.conv2d(skip_s3_1, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s3_2'), train=is_training, name='bn_skip_s3_2')
        skip_s3_out = tf.nn.relu(h1_out_og+skip_s3_2)
        h7_out = tf.cond(tf.greater(epoch, 12), lambda: h7_out+skip_s3_out, lambda: h7_out)

        h8_1 = tf.nn.relu(self.batch_norm(self.deconv2d(h7_out, 16, kernel=(3,3), strides=(1, 1), use_bias = True, name='h8_1'), train=is_training, name='bn_h8_1'))
        h8_2 = self.batch_norm(self.deconv2d(h8_1, 16, kernel=(3,3), strides=(1, 1), use_bias = True, name='h8_2'), train=is_training, name='bn_h8_2')
        h8_out = tf.nn.relu(h7_out+h8_2)
        h8_out = tf.image.resize_nearest_neighbor(h8_out, (256, 256))

        skip_s4_1 = tf.nn.relu(self.batch_norm(self.conv2d(c1_og, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s4_1'), train=is_training, name='bn_skip_s4_1'))
        skip_s4_2 = self.batch_norm(self.conv2d(skip_s4_1, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='skip_s4_2'), train=is_training, name='bn_skip_s4_2')
        skip_s4_out = tf.nn.relu(c1_og+skip_s4_2)
        h8_out = tf.cond(tf.greater(epoch, 14), lambda: h8_out+skip_s4_out, lambda: h8_out)

        c2_1 = tf.nn.relu(self.batch_norm(self.conv2d(h8_out, 16, kernel=(3,3), strides=(1,1), use_bias=True, name='c2_1'), train=is_training, name='bn_c2_1'))
        c2_2 = tf.nn.relu(self.batch_norm(self.conv2d(c2_1, 16, kernel=(1,1), strides=(1,1), use_bias=True, name='c2_2'), train=is_training, name='bn_c2_2'))
        c2_3 = tf.nn.relu(self.batch_norm(self.conv2d(c2_2, 16, kernel=(1,1), strides=(1,1), use_bias=True, name='c2_3'), train=is_training, name='bn_c2_3'))
        c2_o = self.conv2d(c2_3, 1, kernel=(1,1), strides=(1,1), use_bias=True, name='c2_o')

        return c2_o, SL_lin2, identity_features

    def discriminator_256(self, images, is_training, reuse = False):
        with tf.variable_scope("discriminator_256") as scope: 
            if(reuse): 
                tf.get_variable_scope().reuse_variables()
            else:
                assert tf.get_variable_scope().reuse == False
        
            dis_conv1 = tf.nn.relu(self.conv2d(images, self.df_dim, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv1_256'))
            dis_conv2 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv1, self.df_dim*2, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv2_256'), train=is_training, name='bn_dis_conv2_256'))
            dis_conv3 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv2, self.df_dim*4, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv3_256'), train=is_training, name='bn_dis_conv3_256'))
            dis_conv4 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv3, self.df_dim*8, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv4_256'), train=is_training, name='bn_dis_conv4_256'))
            dis_conv5 = self.conv2d(dis_conv4, 1, kernel=(4,4), strides=(1, 1), use_bias = True, name='dis_conv5_256')
       
            return tf.nn.sigmoid(dis_conv5), dis_conv5

    def discriminator_128(self, images, is_training, reuse = False):
        with tf.variable_scope("discriminator_128") as scope: 
            if(reuse): 
                tf.get_variable_scope().reuse_variables()
            else:
                assert tf.get_variable_scope().reuse == False
        
            dis_conv1 = tf.nn.relu(self.conv2d(images, self.df_dim, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv1_128'))
            dis_conv2 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv1, self.df_dim*2, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv2_128'), train=is_training, name='bn_dis_conv2_128'))
            dis_conv3 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv2, self.df_dim*4, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv3_128'), train=is_training, name='bn_dis_conv3_128'))
            dis_conv4 = tf.nn.relu(self.batch_norm(self.conv2d(dis_conv3, self.df_dim*8, kernel=(4,4), strides=(2, 2), use_bias = True, name='dis_conv4_128'), train=is_training, name='bn_dis_conv4_128'))
            dis_conv5 = self.conv2d(dis_conv4, 1, kernel=(4,4), strides=(1, 1), use_bias = True, name='dis_conv5_128')
       
            return tf.nn.sigmoid(dis_conv5), dis_conv5

    def log10(self, x):
        numerator = tf.log(x)
        denominator = tf.log(tf.constant(10, dtype=numerator.dtype))
        return numerator / denominator

    def tf_repeat(self, arr, repeats):
        arr = tf.expand_dims(arr, 1)
        max_repeats = tf.reduce_max(repeats)
        tile_repeats = tf.concat([[1], [max_repeats], tf.ones([tf.rank(arr) - 2], dtype=tf.int32)], axis=0)
        arr_tiled = tf.tile(arr, tile_repeats)
        mask = tf.less(tf.range(max_repeats), tf.expand_dims(repeats, 1))
        result = tf.boolean_mask(arr_tiled, mask)
        return result

    def conv2d(self, input_, output_dim, kernel = (3,3), strides=(2, 2), use_bias=True, stddev=0.02, name="conv2d"):
        with tf.variable_scope(name):
            w = tf.get_variable('w', [kernel[0], kernel[1], input_.get_shape()[-1], output_dim], initializer=tf.truncated_normal_initializer(stddev=stddev))
            conv = tf.nn.conv2d(input_, w, strides=[1, strides[0], strides[1], 1], padding='SAME')

            if use_bias:
                biases = tf.get_variable('biases', [output_dim], initializer=tf.constant_initializer(0.0))
                conv = tf.reshape(tf.nn.bias_add(conv, biases), self.get_shape(conv))

            return conv


    def deconv2d(self, input_, output_shape, kernel=(3,3), strides=(2,2), stddev=0.02, use_bias = True, name="deconv2d", with_w=False, reuse = False):
        with tf.variable_scope(name):
            if type(output_shape) == list():
                output_shape = output_shape[-1]
            deconv = tf.layers.conv2d_transpose(input_, output_shape, kernel, strides, padding='SAME', data_format='channels_last', activation=None, use_bias=use_bias, 
                                                kernel_initializer=tf.random_normal_initializer(stddev=stddev), bias_initializer=tf.zeros_initializer(),
                                                trainable=True, name=name, reuse=reuse)

            return deconv

    def linear(self, input_, output_size, scope="Linear", stddev=0.02, bias_start=0.0, relu=True):
        if len(input_.shape) > 2:
            input_ = tf.reshape(input_, [-1, np.prod([d.value for d in input_.shape[1:]])])
    
        with tf.variable_scope(scope or "Linear"):
            matrix = tf.get_variable("Matrix", [input_.shape[1].value, output_size], tf.float32,
                                     tf.random_normal_initializer(stddev=stddev))
            print(matrix.name, self.get_shape(matrix))
            bias = tf.get_variable("bias", [output_size],
                initializer=tf.constant_initializer(bias_start))
            lin = tf.matmul(input_, matrix) + bias
            if relu:
                lin = tf.nn.relu(lin)
            return lin
   
    def batch_norm(self, x, train=True, reuse=False, trainable=True, epsilon=1e-5, momentum = 0.9, name="batch_norm"):
        return tf.contrib.layers.batch_norm(x,
                      decay=momentum, 
                      updates_collections=None,
                      epsilon=epsilon,
                      fused=True,
                      scale=True,
                      trainable=trainable,
                      is_training=train,
                      scope=name)

    def get_shape(self, tensor):
        static_shape = tensor.shape.as_list()
        dynamic_shape = tf.unstack(tf.shape(tensor))
        dims = [s[1] if s[0] is None else s[0] for s in zip(static_shape, dynamic_shape)]
        return dims

    @property
    def model_dir(self):
        return "" 
      
    def save(self, checkpoint_dir, step):
        model_name = "RelightNet.model"
        checkpoint_dir = os.path.join(checkpoint_dir, self.model_dir)

        if not os.path.exists(checkpoint_dir):
            os.makedirs(checkpoint_dir)

        self.saver.save(self.sess, os.path.join(checkpoint_dir, model_name), global_step=step, write_meta_graph=False)
        print(" Saved checkpoint %s-%d" % (os.path.join(checkpoint_dir, model_name), step))

    def load(self, checkpoint_dir, saver):
        import re
        print(" [*] Reading checkpoints...")
        checkpoint_dir = os.path.join(checkpoint_dir, self.model_dir)

        ckpt = tf.train.get_checkpoint_state(checkpoint_dir)
        if ckpt and ckpt.model_checkpoint_path:
            ckpt_name = os.path.basename(ckpt.model_checkpoint_path)

            print(os.path.join(checkpoint_dir, ckpt_name))
            saver.restore(self.sess, os.path.join(checkpoint_dir, ckpt_name))
            counter = int(next(re.finditer("(\d+)(?!.*\d)",ckpt_name)).group(0))
            print(" [*] Success to read {}".format(ckpt_name))


            return True, counter
        else:
            print(" [*] Failed to find a checkpoint")

            return False, 0

    def load_checkpoint(self, saver, ckpt_file):
        try:
            saver.restore(self.sess, ckpt_file)
            print(" [*] Success to read {}".format(ckpt_file))
        except Exception as e:
            print(e)
            self.load(ckpt_file)

    def load_all_training_data(self):
        all_input_luminance_DPR = np.zeros((123800, 256, 256, 1))
        all_source_lighting_coefficients_DPR = np.zeros((123800, 9))
        all_border_weights_DPR = np.zeros((123800, 256, 256, 1))
        all_input_luminance_Yale = np.zeros((16380, 256, 256, 1))
        all_source_lighting_coefficients_Yale = np.zeros((16380, 9))
        all_border_weights_Yale = np.zeros((16380, 256, 256, 1))

        all_images_DPR = sorted(os.listdir('MP_data/DPR_training_images_cropped_full/'))

        border_weights_DPR = sorted(os.listdir('MP_data/DPR_training_border_weights_full/'))

        all_lighting_files_DPR = sorted(os.listdir('MP_data/DPR_training_lighting_full/'))

        for i in range(len(all_images_DPR)):
            print(i)
            print(all_images_DPR[i])
            curr_border_weights = scipy.io.loadmat('MP_data/DPR_training_border_weights_full/'+border_weights_DPR[i])
            all_border_weights_DPR[i, :, :, :] = np.reshape(curr_border_weights['border_weights'], (256, 256, 1))
            
            curr_RGB_image = imageio.imread('MP_data/DPR_training_images_cropped_full/'+all_images_DPR[i])/255.0
            all_input_luminance_DPR[i, :, :, :] = np.reshape(0.299*curr_RGB_image[:, :, 0]+0.587*curr_RGB_image[:, :, 1]+0.114*curr_RGB_image[:, :, 2], (256, 256, 1))

            name_parts = all_images_DPR[i].split('_')
            curr_num = name_parts[1].split('.')[0][1]
            
            curr_shadow_map = cv2.imread('MP_data/DPR_shadow_maps/'+name_parts[0]+'_'+curr_num+'.bmp')
            curr_mask = np.reshape(np.array((curr_shadow_map[:, :, 0] == 0), dtype=int), (256, 256, 1))
            if(np.sum(curr_mask) > 2000):
                ambient_term = np.sum(curr_mask*all_input_luminance_DPR[i, :, :, :])/(np.sum(curr_mask)+0.0000001)
            else: 
                ambient_term = 0.2986
            print(ambient_term)
            
            all_source_lighting_coefficients_DPR[i, :] = scipy.io.loadmat('MP_data/DPR_training_lighting_full/'+all_lighting_files_DPR[i])['SH_coefficients']
            all_source_lighting_coefficients_DPR[i, 0] += ambient_term
            print(all_source_lighting_coefficients_DPR[i, 0])

        all_images_Yale = sorted(os.listdir('MP_data/Yale_cropped_images/'))

        border_weights_Yale = sorted(os.listdir('MP_data/border_weights_Yale/'))

        Yale_lightings = scipy.io.loadmat('MP_data/Yale_SH_coefficients_corrected.mat')['SH_coeffs']

        for i in range(len(all_images_Yale)):
            print(i)
            print(all_images_Yale[i])
            curr_border_weights = scipy.io.loadmat('MP_data/border_weights_Yale/'+border_weights_Yale[i])
            all_border_weights_Yale[i, :, :, :] = np.reshape(curr_border_weights['border_weights'], (256, 256, 1))
            
            curr_image = imageio.imread('MP_data/Yale_cropped_images/'+all_images_Yale[i])/255.0
            all_input_luminance_Yale[i, :, :, :] = np.reshape(0.299*curr_image+0.587*curr_image+0.114*curr_image, (256, 256, 1))

            name_parts = all_images_Yale[i].split('.')
            curr_shadow_map = cv2.imread('MP_data/Yale_shadow_maps_proper_naming_convention/'+name_parts[0]+'.pgm.png')
            curr_mask = np.reshape(np.array((curr_shadow_map[:, :, 0] == 0), dtype=int), (256, 256, 1))
            if(np.sum(curr_mask) > 2000):
                ambient_term = np.sum(curr_mask*all_input_luminance_Yale[i, :, :, :])/(np.sum(curr_mask)+0.0000001)
            else:
                ambient_term = 0.0704
            print(ambient_term)
            
            source_lighting_idx = i % 65
            all_source_lighting_coefficients_Yale[i, :] = Yale_lightings[source_lighting_idx, :]
            all_source_lighting_coefficients_Yale[i, 0] += ambient_term
            print(all_source_lighting_coefficients_Yale[i, 0])
        
        return all_input_luminance_DPR, all_source_lighting_coefficients_DPR, all_border_weights_DPR, all_input_luminance_Yale, all_source_lighting_coefficients_Yale, all_border_weights_Yale

    def train(self):
        (all_input_luminance_DPR, all_source_lighting_coefficients_DPR, all_border_weights_DPR, all_input_luminance_Yale, all_source_lighting_coefficients_Yale, all_border_weights_Yale) = self.load_all_training_data()
        max_epoch = 1000
        self.g_vars = [var for var in tf.global_variables() if ('dis' not in var.name)]
        self.d_vars_256 = [var for var in tf.global_variables() if ('discriminator_256' in var.name)]  
        self.d_vars_128 = [var for var in tf.global_variables() if ('discriminator_128' in var.name)] 
        self.d_train_op_256 = tf.train.AdamOptimizer(self.lr).minimize(self.total_d_loss_256, var_list=self.d_vars_256)
        self.d_train_op_128 = tf.train.AdamOptimizer(self.lr).minimize(self.total_d_loss_128, var_list=self.d_vars_128)
        self.g_train_op = tf.train.AdamOptimizer(self.lr).minimize(self.total_g_loss, var_list=self.g_vars)
        init_op = tf.initialize_all_variables()
        self.sess.run(init_op)

        self.saver = tf.train.Saver(keep_checkpoint_every_n_hours=1, max_to_keep = 1000)
        self.checkpoint_dir = 'DPR_and_Yale_training_checkpoint_nonlinear_relighting_SH_gradual_skip_SSIM_ratio_image_log_loss_using_SSIM_loss_Yuv_shadow_map_loss_contrast_based_border_weights_with_corrected_patchgan_much_smaller_weights_larger_L1_losses_MR_dis/'

        batch_list = np.arange(int(123800/5))
        batch_order = np.arange(5)

        could_load, checkpoint_counter = self.load(self.checkpoint_dir, self.saver)
        if could_load:
            epoch0 = checkpoint_counter + 1
            print(" [*] Load SUCCESS")
        else:
            epoch0 = 1
            print(" [!] Load failed...")

        num_batches = 5000
        for epoch in xrange(epoch0, max_epoch):
            np.random.shuffle(batch_list)
            total_g_loss_epoch_DPR = 0.0
            total_d_loss_256_epoch_DPR = 0.0
            total_d_loss_128_epoch_DPR = 0.0
            target_ratio_loss_epoch_DPR = 0.0
            source_lighting_loss_epoch_DPR = 0.0
            image_gradient_loss_epoch_DPR = 0.0
            consistency_loss_identity_epoch_DPR = 0.0
            SSIM_loss_epoch_DPR = 0.0
            source_shadow_border_loss_epoch_DPR = 0.0
            target_shadow_border_loss_epoch_DPR = 0.0
            g_loss_epoch_DPR = 0.0
            g_loss_256_epoch_DPR = 0.0
            g_loss_128_epoch_DPR = 0.0
            d_loss_256_real_epoch_DPR = 0.0
            d_loss_256_fake_epoch_DPR = 0.0
            d_loss_128_real_epoch_DPR = 0.0
            d_loss_128_fake_epoch_DPR = 0.0

            total_g_loss_epoch_Yale = 0.0
            total_d_loss_256_epoch_Yale = 0.0
            total_d_loss_128_epoch_Yale = 0.0
            target_ratio_loss_epoch_Yale = 0.0
            source_lighting_loss_epoch_Yale = 0.0
            image_gradient_loss_epoch_Yale = 0.0
            consistency_loss_identity_epoch_Yale = 0.0
            SSIM_loss_epoch_Yale = 0.0
            source_shadow_border_loss_epoch_Yale = 0.0
            target_shadow_border_loss_epoch_Yale = 0.0
            g_loss_epoch_Yale = 0.0
            g_loss_256_epoch_Yale = 0.0
            g_loss_128_epoch_Yale = 0.0
            d_loss_256_real_epoch_Yale = 0.0
            d_loss_256_fake_epoch_Yale = 0.0
            d_loss_128_real_epoch_Yale = 0.0
            d_loss_128_fake_epoch_Yale = 0.0
          
            for i in range(num_batches):
                np.random.shuffle(batch_order)
                curr_source_lighting = all_source_lighting_coefficients_DPR[(batch_list[i]*self.batch_size):((batch_list[i]+1)*self.batch_size)]
                curr_input_luminance = all_input_luminance_DPR[(batch_list[i]*self.batch_size):((batch_list[i]+1)*self.batch_size)]
                curr_input_luminance[curr_input_luminance < 0.01] = 0.01
                tmp_target_luminance = curr_input_luminance[batch_order]
                curr_input_luminance = curr_input_luminance**(1.0/2.2)
                curr_border_weights = all_border_weights_DPR[(batch_list[i]*self.batch_size):((batch_list[i]+1)*self.batch_size)]

                curr_target_luminance = curr_input_luminance[batch_order]
                        
                ffeed_dict={self.input_luminance_ph: curr_input_luminance, \
                            self.gt_ratio_ph: (curr_target_luminance/curr_input_luminance), \
                            self.source_lighting_ph: curr_source_lighting, \
                            self.target_lighting_ph: curr_source_lighting[batch_order], \
                            self.source_border_weights_ph: curr_border_weights, \
                            self.target_border_weights_ph: curr_border_weights[batch_order], \
                            self.target_luminance_ph: tmp_target_luminance, \
                            self.gan_loss_scale_factor_ph: 0.01, \
                            self.epoch_ph: epoch}
                _, _ = self.sess.run([self.d_train_op_256, self.d_train_op_128], feed_dict = ffeed_dict)
                _, total_g_loss, total_d_loss_256, total_d_loss_128, target_ratio_loss, source_lighting_loss, image_gradient_loss, consistency_loss_identity, SSIM_loss, source_shadow_border_loss, target_shadow_border_loss, border_weights_source, border_weights_target, g_loss, g_loss_256, d_loss_real_256, d_loss_fake_256, g_loss_128, d_loss_real_128, d_loss_fake_128 = self.sess.run([self.g_train_op, self.total_g_loss, self.total_d_loss_256, self.total_d_loss_128, self.target_ratio_loss, self.source_lighting_loss, self.image_gradient_loss, self.consistency_loss_identity, self.SSIM_loss, self.source_shadow_border_loss, self.target_shadow_border_loss, self.source_border_weights_ph, self.target_border_weights_ph, self.g_loss, self.g_loss_256, self.d_loss_real_256, self.d_loss_fake_256, self.g_loss_128, self.d_loss_real_128, self.d_loss_fake_128], feed_dict=ffeed_dict)
                total_g_loss_epoch_DPR += total_g_loss
                total_d_loss_256_epoch_DPR += total_d_loss_256
                total_d_loss_128_epoch_DPR += total_d_loss_128
                target_ratio_loss_epoch_DPR += target_ratio_loss
                source_lighting_loss_epoch_DPR += source_lighting_loss
                image_gradient_loss_epoch_DPR += image_gradient_loss
                consistency_loss_identity_epoch_DPR += consistency_loss_identity
                SSIM_loss_epoch_DPR += SSIM_loss
                source_shadow_border_loss_epoch_DPR += source_shadow_border_loss
                target_shadow_border_loss_epoch_DPR += target_shadow_border_loss
                g_loss_epoch_DPR += g_loss
                g_loss_256_epoch_DPR += g_loss_256
                g_loss_128_epoch_DPR += g_loss_128
                d_loss_256_real_epoch_DPR += d_loss_real_256
                d_loss_256_fake_epoch_DPR += d_loss_fake_256
                d_loss_128_real_epoch_DPR += d_loss_real_128
                d_loss_128_fake_epoch_DPR += d_loss_fake_128
                
                print("DPR Epoch: "+str(epoch)+", Batch: "+str(i)+", total g loss: "+str(total_g_loss)+", total d loss 256: "+str(total_d_loss_256)+", total d loss 128: "+str(total_d_loss_128)+", target ratio loss: "+str(target_ratio_loss)+", source lighting loss: "+str(source_lighting_loss)+", image gradient loss: "+str(image_gradient_loss)+", consistency loss identity: "+str(consistency_loss_identity)+", SSIM loss: "+str(SSIM_loss)+", source shadow border loss: "+str(source_shadow_border_loss)+", target shadow border loss: "+str(target_shadow_border_loss)+", g loss: "+str(g_loss)+", g loss 256: "+str(g_loss_256)+", d loss real 256: "+str(d_loss_real_256)+", d loss fake 256: "+str(d_loss_fake_256)+", g loss 128: "+str(g_loss_128)+", d loss real 128: "+str(d_loss_real_128)+", d loss fake 128: "+str(d_loss_fake_128))
                print(np.sum(border_weights_source))
                print(np.sum(border_weights_target))

                random_subject = randrange(252)
                random_source_lightings = np.zeros((self.batch_size), dtype=int)
                random_target_lightings = np.zeros((self.batch_size), dtype=int)
                for j in range(self.batch_size):
                    random_source_lightings[j] = randrange(65)
                    random_target_lightings[j] = randrange(65)
                random_source_indices = random_source_lightings+random_subject*65
                random_target_indices = random_target_lightings+random_subject*65
                curr_source_lighting = all_source_lighting_coefficients_Yale[random_source_indices]
                curr_target_lighting = all_source_lighting_coefficients_Yale[random_target_indices]
                curr_input_luminance = all_input_luminance_Yale[random_source_indices]
                curr_input_luminance[curr_input_luminance < 0.01] = 0.01
                curr_input_luminance = curr_input_luminance**(1.0/2.2)
                curr_target_luminance = all_input_luminance_Yale[random_target_indices]
                curr_target_luminance[curr_target_luminance < 0.01] = 0.01
                tmp_target_luminance = curr_target_luminance
                curr_target_luminance = curr_target_luminance**(1.0/2.2)
                curr_border_weights = all_border_weights_Yale[random_source_indices]
                curr_target_border_weights = all_border_weights_Yale[random_target_indices]
    
                ffeed_dict={self.input_luminance_ph: curr_input_luminance, \
                            self.gt_ratio_ph: (curr_target_luminance/curr_input_luminance), \
                            self.source_lighting_ph: curr_source_lighting, \
                            self.target_lighting_ph: curr_target_lighting, \
                            self.source_border_weights_ph: curr_border_weights, \
                            self.target_border_weights_ph: curr_target_border_weights, \
                            self.target_luminance_ph: tmp_target_luminance, \
                            self.gan_loss_scale_factor_ph: 0.01, \
                            self.epoch_ph: epoch}
                _, _ = self.sess.run([self.d_train_op_256, self.d_train_op_128], feed_dict = ffeed_dict)
                _, total_g_loss, total_d_loss_256, total_d_loss_128, target_ratio_loss, source_lighting_loss, image_gradient_loss, consistency_loss_identity, SSIM_loss, source_shadow_border_loss, target_shadow_border_loss, border_weights_source, border_weights_target, g_loss, g_loss_256, d_loss_real_256, d_loss_fake_256, g_loss_128, d_loss_real_128, d_loss_fake_128 = self.sess.run([self.g_train_op, self.total_g_loss, self.total_d_loss_256, self.total_d_loss_128, self.target_ratio_loss, self.source_lighting_loss, self.image_gradient_loss, self.consistency_loss_identity, self.SSIM_loss, self.source_shadow_border_loss, self.target_shadow_border_loss, self.source_border_weights_ph, self.target_border_weights_ph, self.g_loss, self.g_loss_256, self.d_loss_real_256, self.d_loss_fake_256, self.g_loss_128, self.d_loss_real_128, self.d_loss_fake_128], feed_dict=ffeed_dict)
                total_g_loss_epoch_Yale += total_g_loss
                total_d_loss_256_epoch_Yale += total_d_loss_256
                total_d_loss_128_epoch_Yale += total_d_loss_128
                target_ratio_loss_epoch_Yale += target_ratio_loss
                source_lighting_loss_epoch_Yale += source_lighting_loss
                image_gradient_loss_epoch_Yale += image_gradient_loss
                consistency_loss_identity_epoch_Yale += consistency_loss_identity
                SSIM_loss_epoch_Yale += SSIM_loss
                source_shadow_border_loss_epoch_Yale += source_shadow_border_loss
                target_shadow_border_loss_epoch_Yale += target_shadow_border_loss
                g_loss_epoch_Yale += g_loss
                g_loss_256_epoch_Yale += g_loss_256
                g_loss_128_epoch_Yale += g_loss_128
                d_loss_256_real_epoch_Yale += d_loss_real_256
                d_loss_256_fake_epoch_Yale += d_loss_fake_256
                d_loss_128_real_epoch_Yale += d_loss_real_128
                d_loss_128_fake_epoch_Yale += d_loss_fake_128
                
                print("Yale Epoch: "+str(epoch)+", Batch: "+str(i)+", total g loss: "+str(total_g_loss)+", total d loss 256: "+str(total_d_loss_256)+", total d loss 128: "+str(total_d_loss_128)+", target ratio loss: "+str(target_ratio_loss)+", source lighting loss: "+str(source_lighting_loss)+", image gradient loss: "+str(image_gradient_loss)+", consistency loss identity: "+str(consistency_loss_identity)+", SSIM loss: "+str(SSIM_loss)+", source shadow border loss: "+str(source_shadow_border_loss)+", target shadow border loss: "+str(target_shadow_border_loss)+", g loss: "+str(g_loss)+", g loss 256: "+str(g_loss_256)+", d loss real 256: "+str(d_loss_real_256)+", d loss fake 256: "+str(d_loss_fake_256)+", g loss 128: "+str(g_loss_128)+", d loss real 128: "+str(d_loss_real_128)+", d loss fake 128: "+str(d_loss_fake_128))
                print(np.sum(border_weights_source))
                print(np.sum(border_weights_target))

            self.save(self.checkpoint_dir, epoch)
            total_g_loss_epoch_DPR /= num_batches
            total_d_loss_256_epoch_DPR /= num_batches
            total_d_loss_128_epoch_DPR /= num_batches
            target_ratio_loss_epoch_DPR /= num_batches
            source_lighting_loss_epoch_DPR /= num_batches
            image_gradient_loss_epoch_DPR /= num_batches
            consistency_loss_identity_epoch_DPR /= num_batches
            SSIM_loss_epoch_DPR /= num_batches
            source_shadow_border_loss_epoch_DPR /= num_batches
            target_shadow_border_loss_epoch_DPR /= num_batches
            g_loss_epoch_DPR /= num_batches
            g_loss_256_epoch_DPR /= num_batches
            g_loss_128_epoch_DPR /= num_batches
            d_loss_256_real_epoch_DPR /= num_batches
            d_loss_256_fake_epoch_DPR /= num_batches
            d_loss_128_real_epoch_DPR /= num_batches
            d_loss_128_fake_epoch_DPR /= num_batches

            loss_curr_epoch = np.zeros((17))
            loss_curr_epoch[0] = total_g_loss_epoch_DPR
            loss_curr_epoch[1] = total_d_loss_256_epoch_DPR
            loss_curr_epoch[2] = total_d_loss_128_epoch_DPR
            loss_curr_epoch[3] = target_ratio_loss_epoch_DPR
            loss_curr_epoch[4] = source_lighting_loss_epoch_DPR
            loss_curr_epoch[5] = image_gradient_loss_epoch_DPR
            loss_curr_epoch[6] = consistency_loss_identity_epoch_DPR
            loss_curr_epoch[7] = SSIM_loss_epoch_DPR
            loss_curr_epoch[8] = source_shadow_border_loss_epoch_DPR
            loss_curr_epoch[9] = target_shadow_border_loss_epoch_DPR
            loss_curr_epoch[10] = g_loss_epoch_DPR
            loss_curr_epoch[11] = g_loss_256_epoch_DPR
            loss_curr_epoch[12] = g_loss_128_epoch_DPR
            loss_curr_epoch[13] = d_loss_256_real_epoch_DPR
            loss_curr_epoch[14] = d_loss_256_fake_epoch_DPR
            loss_curr_epoch[15] = d_loss_128_real_epoch_DPR
            loss_curr_epoch[16] = d_loss_128_fake_epoch_DPR
            
            output_mat = {}
            output_mat['losses'] = loss_curr_epoch
            scipy.io.savemat('DPR_and_Yale_loss_curves_nonlinear_relighting_SH_model_gradual_skip_SSIM_ratio_image_log_loss_using_SSIM_loss_Yuv_shadow_map_loss_contrast_based_border_weights_with_corrected_patchgan_much_smaller_weights_larger_L1_losses_MR_dis_DPR_losses/losses_epoch'+str(epoch)+'.mat', output_mat)

            total_g_loss_epoch_Yale /= num_batches
            total_d_loss_256_epoch_Yale /= num_batches
            total_d_loss_128_epoch_Yale /= num_batches
            target_ratio_loss_epoch_Yale /= num_batches
            source_lighting_loss_epoch_Yale /= num_batches
            image_gradient_loss_epoch_Yale /= num_batches
            consistency_loss_identity_epoch_Yale /= num_batches
            SSIM_loss_epoch_Yale /= num_batches
            source_shadow_border_loss_epoch_Yale /= num_batches
            target_shadow_border_loss_epoch_Yale /= num_batches
            g_loss_epoch_Yale /= num_batches
            g_loss_256_epoch_Yale /= num_batches
            g_loss_128_epoch_Yale /= num_batches
            d_loss_256_real_epoch_Yale /= num_batches
            d_loss_256_fake_epoch_Yale /= num_batches
            d_loss_128_real_epoch_Yale /= num_batches
            d_loss_128_fake_epoch_Yale /= num_batches

            loss_curr_epoch = np.zeros((17))
            loss_curr_epoch[0] = total_g_loss_epoch_Yale
            loss_curr_epoch[1] = total_d_loss_256_epoch_Yale
            loss_curr_epoch[2] = total_d_loss_128_epoch_Yale
            loss_curr_epoch[3] = target_ratio_loss_epoch_Yale
            loss_curr_epoch[4] = source_lighting_loss_epoch_Yale
            loss_curr_epoch[5] = image_gradient_loss_epoch_Yale
            loss_curr_epoch[6] = consistency_loss_identity_epoch_Yale
            loss_curr_epoch[7] = SSIM_loss_epoch_Yale
            loss_curr_epoch[8] = source_shadow_border_loss_epoch_Yale
            loss_curr_epoch[9] = target_shadow_border_loss_epoch_Yale
            loss_curr_epoch[10] = g_loss_epoch_Yale
            loss_curr_epoch[11] = g_loss_256_epoch_Yale
            loss_curr_epoch[12] = g_loss_128_epoch_Yale
            loss_curr_epoch[13] = d_loss_256_real_epoch_Yale
            loss_curr_epoch[14] = d_loss_256_fake_epoch_Yale
            loss_curr_epoch[15] = d_loss_128_real_epoch_Yale
            loss_curr_epoch[16] = d_loss_128_fake_epoch_Yale
            
            output_mat = {}
            output_mat['losses'] = loss_curr_epoch
            scipy.io.savemat('DPR_and_Yale_loss_curves_nonlinear_relighting_SH_model_gradual_skip_SSIM_ratio_image_log_loss_using_SSIM_loss_Yuv_shadow_map_loss_contrast_based_border_weights_with_corrected_patchgan_much_smaller_weights_larger_L1_losses_MR_dis_Yale_losses/losses_epoch'+str(epoch)+'.mat', output_mat)
                
            

def main():
    gpu_options = tf.GPUOptions(visible_device_list='0', per_process_gpu_memory_fraction = 0.99, allow_growth = True)

    with tf.Session(config=tf.ConfigProto(allow_soft_placement=True, log_device_placement=False, gpu_options=gpu_options)) as sess:
        RN = RelightNet(sess)
        RN.train()
            
            

if __name__ == '__main__':
    main()
