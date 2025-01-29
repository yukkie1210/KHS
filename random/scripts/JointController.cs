using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace InverseKinematics{
    public class JointController : MonoBehaviour
    {
        //robot
        private GameObject[] joint = new GameObject[6];
        private GameObject[] arm = new GameObject[6];
        private float[] armL = new float[6];
        private Vector3[] angle = new Vector3[6];
        private Vector3 pos;
        private Vector3 rot;

        //UI
        private GameObject[] slider = new GameObject[6];
        private float[] sliderVal = new float[6];
        private float[] prevSliderVal = new float[6];
        private GameObject[] angText = new GameObject[6];
        private GameObject[] posText = new GameObject[6];
        void Start()
        {
            for (int i = 0; i < joint.Length; i++){
                //robot
                joint[i] = GameObject.Find("Joint_" + i.ToString());
                arm[i] = GameObject.Find("Arm_" + i.ToStringl());
                armL[i] = arm[i].transform.localScale.z;

                //UI
                slider[i] = GameObject.Find("Slider_" + i.ToString());
                sliderVal[i] = slider[i].GetComponent<Slider>().value;
                posText[i] = GameObject.Find("Ref_" + i.ToString());
                angText[i] = GameObject.Find("Angle_" + i.ToString());
            }
        }

        void Update()
        {
            for (int i = 0; i < joint.Length; i++){
                sliderVal[i] = slider[i].GetComponent<Slider>().value;
            }
            pos.x = sliderVal[0];
            pos.y = sliderVal[1];
            pos.z = sliderVal[2];
            rot.x = sliderVal[3];
            rot.y = sliderVal[4];
            rot.z = sliderVal[5];

            //calc Joint_4 position
            float endL = armL[4] + armL[5];
            Quaternion q = Quaternion.Euler(rot.x, rot.y, rot.z);
            Vector3 v = q * new Vector3(0, 0, endL);
            float x = pos.x - (q*v).x;
            float y = pos.y - (q*v).y;
            float z = pos.z - (q*v).z;

            //calc Angle_0, 1, 2
            angle[0].x = Mathf.Atan2(y, x);
        }
    }
}
