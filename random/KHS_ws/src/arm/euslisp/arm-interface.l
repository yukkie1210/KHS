#!/usr/bin/env roseus

(ros::roseus-add-msgs "std_msgs")
(ros::roseus-add-msgs "sensor_msgs")
(load "package://kxreus/euslisp/kxr-interface.l")
(load "package://arm/euslisp/arm.l")


(defun calc-yaw-pitch-roll (linear-acceleration)
  (let* ((x (send linear-acceleration :x))
         (y (send linear-acceleration :y))
         (z (send linear-acceleration :z))
         ;; 各角度の計算 (radian単位)
         (roll (atan (- x) (sqrt (+ (* y y) (* z z)))))
         (pitch (atan y z))
         (yaw 0))  ;; Yawは加速度だけでは計算できないので、0を仮定

    ;; 結果をdegreeにして次の順序で返す (roll, pitch, yaw)
    (list (rad2deg roll) (rad2deg pitch) (rad2deg yaw))))


(defclass arm-interface
  :super kxr-interface
  :slots (atom-s3-button-click-count atom-s3-additional-info-topic-name imu-raw-msg))

(defmethod arm-interface
  (:init (robot &rest args &key (namespace nil)
                (type :default-controller)
                (controller-name "fullbody_controller") &allow-other-keys)
   (send-super* :init robot :type type
                :namespace namespace
                :controller-name controller-name
                :input-joint-names (list "joint1"
                                         "joint2"
                                         "joint3"
                                         "joint4"
                                         "joint5"
                                         "joint6")
                args)
   (dolist (l (list
               (cons :arm-controller "arm_controller/follow_joint_trajectory")))
     (let ((type (car l))
           (name (cdr l))
           action)
       (setq action (find-if #'(lambda (ac) (string= name (send ac :name)))
                             controller-actions))
       (setf (gethash type controller-table) (list action))
       ))

   #|(setq atom-s3-button-click-count 0)
   (setq atom-s3-additional-info-topic-name (if namespace (format nil "~A/atom_s3_additional_info" namespace)
                                                                          "/atom_s3_additional_info"))
   (ros::advertise atom-s3-additional-info-topic-name std_msgs::String 1)
   (ros::subscribe (if namespace (format nil "~A/atom_s3_button_state" namespace)
                       "/atom_s3_button_state") std_msgs::Int32
                       #'send self :atom-s3-button-state-callback :groupname groupname)
   (setq imu-raw-msg (instance sensor_msgs::Imu :init))
   (ros::subscribe (if namespace (format nil "~A/imu_publisher/imu" namespace)
                       "/imu_publisher/imu") sensor_msgs::Imu
                       #'send self :atom-s3-imu-callback :groupname groupname)|#
   (ros::ros-info "Initializing arm-interface"))
  (:state (&rest args) ;; overwrite robot-interface
          (case (car args)
            (:atom-s3-button
             (let ((button-value atom-s3-button-click-count))
               (setq atom-s3-button-click-count 0)
               button-value))
            (:imu
             imu-raw-msg)
            (:roll-pitch-yaw
             (calc-yaw-pitch-roll (send imu-raw-msg :linear_acceleration)))
            (t
             (send-super* :state args))
            ))
  (:atom-s3-button-state-callback (msg)
                                  (when (> (send msg :data) 0)
                                    (setq atom-s3-button-click-count (send msg :data))))
  (:atom-s3-imu-callback (msg)
                         (setq imu-raw-msg msg))
  (:publish-atom-s3-string (text)
    (ros::publish atom-s3-additional-info-topic-name
                  (instance std_msgs::String :init :data text)))
  (:arm-controller
   ()
   (list
	  (list
	   (cons :controller-action "arm_controller/follow_joint_trajectory")
	   (cons :controller-state "arm_controller/state")
	   (cons :action-type control_msgs::FollowJointTrajectoryAction)
	   (cons :joint-names (list "1st_joint" "2nd_joint"
                              "3rd_joint" "4th_joint"
                              "5th_joint" "6th_joint")))))
  (:default-controller
      ()
      (append
       (send self :arm-controller)))
  )



(defun arm-init (&key
                   (namespace nil)
                   (without-ri nil)
                   (create-viewer t)
                   (controller-name "fullbody_controller"))
  (unless (boundp '*arm*)
    (setq *arm* (arm)))
  (unless without-ri
    (unless (ros::ok) (ros::roseus "arm_eus_interface"))
    (unless (boundp '*ri*)
      (setq *ri* (instance arm-interface :init *arm* :namespace namespace
                           :controller-name controller-name)))

    (ros::spin-once)
    (send *ri* :spin-once)
    (send *arm* :angle-vector (send *ri* :state :potentio-vector)))
  (when create-viewer (objects (list *arm*))))

(defun servo-off-sequence ()
  (send *ri* :angle-vector (send *arm* :reset-pose) 5000)
  (send *ri* :wait-interpolation)
  (send *ri* :servo-off))
