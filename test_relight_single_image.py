import os
import cv2
from math import floor, sqrt, pi
import tensorflow as tf
import numpy as np
import sys
import imageio

class RelightNet(object):
    def __init__(self, sess):
        self.sess = sess
        self.batch_size = 1
        self.img_height = 256
        self.img_width = 256
        self.SH_dim = 9
        self.lr = 0.0001
        self.df_dim = 64
        self.build_model()

    def build_model(self):
        self.input_luminance_ph = tf.placeholder(tf.float32, [self.batch_size, self.img_height, self.img_width, 1], name='input_luminance')
        self.target_lighting_ph = tf.placeholder(tf.float32, [self.batch_size, self.SH_dim], name='SH_target')
        self.epoch_ph = tf.placeholder(tf.float32, [], name='epoch_num')

        (self.predicted_ratio, self.predicted_source_lighting, self.identity_features) = self.relighting_UNET(self.input_luminance_ph, self.target_lighting_ph, False, self.epoch_ph)

        self.predicted_ratio = tf.clip_by_value(self.predicted_ratio, clip_value_min=0.0000000001, clip_value_max=10000000000)

        _, _ = self.discriminator_256(self.input_luminance_ph, is_training=False)
        self.input_luminance_128 = tf.image.resize_nearest_neighbor(self.input_luminance_ph, (128, 128))
        _, _ = self.discriminator_128(self.input_luminance_128, is_training=False)

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

    def get_shading(self, normal, SH):
        '''
	   get shading based on normals and SH
	   normal is Nx3 matrix
	   SH: 9 x m vector
	   return Nxm vector, where m is the number of returned images
        '''
        sh_basis = self.SH_basis(normal)
        shading = np.matmul(sh_basis, SH)
        return shading

    def SH_basis(self, normal):
        '''
           get SH basis based on normal
           normal is a Nx3 matrix
           return a Nx9 matrix
           The order of SH here is:
           1, Y, Z, X, YX, YZ, 3Z^2-1, XZ, X^2-y^2
        '''
        numElem = normal.shape[0]

        norm_X = normal[:,0]
        norm_Y = normal[:,1]
        norm_Z = normal[:,2]

        sh_basis = np.zeros((numElem, 9))
        att= np.pi*np.array([1, 2.0/3.0, 1/4.0])
        sh_basis[:,0] = 0.5/np.sqrt(np.pi)*att[0]

        sh_basis[:,1] = np.sqrt(3)/2/np.sqrt(np.pi)*norm_Y*att[1]
        sh_basis[:,2] = np.sqrt(3)/2/np.sqrt(np.pi)*norm_Z*att[1]
        sh_basis[:,3] = np.sqrt(3)/2/np.sqrt(np.pi)*norm_X*att[1]

        sh_basis[:,4] = np.sqrt(15)/2/np.sqrt(np.pi)*norm_Y*norm_X*att[2]
        sh_basis[:,5] = np.sqrt(15)/2/np.sqrt(np.pi)*norm_Y*norm_Z*att[2]
        sh_basis[:,6] = np.sqrt(5)/4/np.sqrt(np.pi)*(3*norm_Z**2-1)*att[2]
        sh_basis[:,7] = np.sqrt(15)/2/np.sqrt(np.pi)*norm_X*norm_Z*att[2]
        sh_basis[:,8] = np.sqrt(15)/4/np.sqrt(np.pi)*(norm_X**2-norm_Y**2)*att[2]
        return sh_basis

    def test(self):
        init_op = tf.initialize_all_variables()
        self.sess.run(init_op)

        self.saver = tf.train.Saver(keep_checkpoint_every_n_hours=1, max_to_keep = 1000)
        self.checkpoint_dir = 'model/'

        could_load, checkpoint_counter = self.load(self.checkpoint_dir, self.saver)
        if could_load:
            epoch0 = checkpoint_counter + 1
            print(" [*] Load SUCCESS")
        else:
            epoch0 = 1
            print(" [!] Load failed...")

        curr_RGB_image = imageio.imread(sys.argv[1])/255.0
       
        curr_RGB_image = cv2.resize(curr_RGB_image, (256, 256))
        curr_input_luminance = np.reshape(0.299*curr_RGB_image[:, :, 0]+0.587*curr_RGB_image[:, :, 1]+0.114*curr_RGB_image[:, :, 2], (256, 256, 1))
        curr_input_luminance[curr_input_luminance < 0.01] = 0.01
        curr_input_luminance = curr_input_luminance**(1.0/2.2)  
        curr_second_channel = np.reshape(-0.14713*curr_RGB_image[:, :, 0]-0.28886*curr_RGB_image[:, :, 1]+0.436*curr_RGB_image[:, :, 2], (256, 256, 1))
        curr_third_channel = np.reshape(0.615*curr_RGB_image[:, :, 0]-0.51499*curr_RGB_image[:, :, 1]-0.10001*curr_RGB_image[:, :, 2], (256, 256, 1))        

        target_lighting_file = sys.argv[2]    
        sh = np.loadtxt(target_lighting_file)
        sh = np.reshape(sh[0:9], (1, 9))
            
        savePath = sys.argv[3]

        # ---------------- create normal for rendering half sphere ------
        img_size = 256
        x = np.linspace(-1, 1, img_size)
        z = np.linspace(1, -1, img_size)
        x, z = np.meshgrid(x, z)

        mag = np.sqrt(x**2 + z**2)
        valid = mag <=1
        y = -np.sqrt(1 - (x*valid)**2 - (z*valid)**2)
        x = x * valid
        y = y * valid
        z = z * valid
        normal = np.concatenate((x[...,None], y[...,None], z[...,None]), axis=2)
        normal = np.reshape(normal, (-1, 3))

        ffeed_dict={self.input_luminance_ph: np.reshape(curr_input_luminance, (1, 256, 256, 1)), self.target_lighting_ph: sh, self.epoch_ph: 51}

        predicted_ratio = self.sess.run([self.predicted_ratio], feed_dict=ffeed_dict)
        predicted_RGB = np.zeros((256, 256, 3))
        predicted_luminance = curr_input_luminance*predicted_ratio
        predicted_luminance = predicted_luminance**2.2
        predicted_RGB[:, :, 2] = np.reshape(predicted_luminance+1.13983*curr_third_channel, (256, 256))
        predicted_RGB[:, :, 1] = np.reshape(predicted_luminance-0.39465*curr_second_channel-0.58060*curr_third_channel, (256, 256))
        predicted_RGB[:, :, 0] = np.reshape(predicted_luminance+2.03211*curr_second_channel, (256, 256))
        predicted_RGB = 255.0*predicted_RGB

        sh = np.squeeze(sh)
        shading = self.get_shading(normal, sh)
        value = np.percentile(shading, 95)
        ind = shading > value
        shading[ind] = value  
        shading = (shading - np.min(shading))/(np.max(shading) - np.min(shading))
        shading = (shading *255.0).astype(np.uint8)
        shading = np.reshape(shading, (256, 256))
        shading = shading * valid

        big_img = np.zeros((256, 768, 3))
        big_img[0:256, 0:256, :] = 255.0*curr_RGB_image[:, :, ::-1]
        big_img[0:256, 256:512, 0] = shading
        big_img[0:256, 256:512, 1] = shading
        big_img[0:256, 256:512, 2] = shading
        big_img[0:256, 512:768, :] = predicted_RGB         

        cv2.imwrite(savePath, big_img)

def main():
    gpu_options = tf.GPUOptions(visible_device_list=sys.argv[4], per_process_gpu_memory_fraction = 0.99, allow_growth = True)

    with tf.Session(config=tf.ConfigProto(allow_soft_placement=True, log_device_placement=False, gpu_options=gpu_options)) as sess:
        RN = RelightNet(sess)
        RN.test()
            
            

if __name__ == '__main__':
    main()
