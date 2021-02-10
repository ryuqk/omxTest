# omxTest
OpenMax decoder Lib for Raspberry PI

Camera -- V4L2 -- OMX -- HDMI -- LCD

# test code

 input: raspi camera (v4l2)

 output: HDMI monitor(OMX)

* Make

>make

* clean

> make clean

* run

>xc_omxPlay
<pre>
args
        -d: device default: /dev/video0
        -H: image height default:640 
        -W: image width default:480 
        -F: image fps default:30 (not work)
        -h: this help 
</pre>
