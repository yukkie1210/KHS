#!/usr/bin/env python3

import rospy
from geometry_msgs.msg import Pose
from sensor_msgs.msg import JointState
from moveit_commander import RobotCommander, PlanningSceneInterface
from moveit_commander.conversions import pose_to_list
from moveit_commander.robot_trajectory import RobotTrajectory

class MoveItIKPublisher:
    def __init__(self):
        # Initialize ROS node
        rospy.init_node('moveit_ik_publisher', anonymous=True)

        # Publishers and Subscribers
        self.pose_sub = rospy.Subscriber('/target_pose', Pose, self.pose_callback)
        self.joint_state_pub = rospy.Publisher('/joint_states', JointState, queue_size=10)

        # Initialize MoveIt
        self.robot = RobotCommander()
        self.scene = PlanningSceneInterface()
        self.group_name = "arm"  # グループ名（MoveIt設定に合わせて変更）
        self.move_group = self.robot.get_group(self.group_name)

        rospy.loginfo("MoveItIKPublisher initialized. Waiting for target poses...")

    def pose_callback(self, target_pose):
        # IK計算を実行
        rospy.loginfo("Received target pose for IK computation.")

        try:
            # IKを解く
            self.move_group.set_pose_target(target_pose)
            plan = self.move_group.plan()  # Plan作成

            # Planが成功した場合
            if plan and len(plan.joint_trajectory.points) > 0:
                joint_values = plan.joint_trajectory.points[0].positions

                # Publish the joint states
                joint_state_msg = JointState()
                joint_state_msg.header.stamp = rospy.Time.now()
                joint_state_msg.name = self.move_group.get_active_joints()
                joint_state_msg.position = joint_values

                self.joint_state_pub.publish(joint_state_msg)
                rospy.loginfo("Joint states published: %s", joint_values)
            else:
                rospy.logwarn("IK solution not found or planning failed.")

        except Exception as e:
            rospy.logerr("Error during IK computation: %s", e)

    def run(self):
        rospy.spin()

if __name__ == '__main__':
    try:
        ik_publisher = MoveItIKPublisher()
        ik_publisher.run()
    except rospy.ROSInterruptException:
        pass
