using UnityEngine;
using UnityEngine.UI;
using MathNet.Numerics.LinearAlgebra.Single;

public class IK : MonoBehaviour
{
    public GetHMDPose getHMDPose;
    public OVRBody body;
    // public OVRHand leftHand;

    //robot
    //joint: 対偶とLinkの組    joint[-1]: Base_OriginとBase_Linkの組
    private const int SERVO_NUM = 7;
    private GameObject[] joint = new GameObject[6];
    private float[] angle = new float[SERVO_NUM];
    private float[] servoAngle = new float[SERVO_NUM];
    //初期のアームの寸法
    private Vector3[] dim = new Vector3[6];
    //対偶の相対位置
    private Vector3[] point = new Vector3[7];
    //回転軸
    private Vector3[] axis = new Vector3[6];
    private Quaternion[] rotation = new Quaternion[6];
    private Quaternion[] wRotation = new Quaternion[6];
    private Vector3 pos;
    private Vector3 rot;
    // private float lambda = 0.1f;
    
    //UI
    // private GameObject[] slider = new GameObject[6];
    // private float[] sliderVal = new float[6];
    // private float[] prevSliderVal = new float[6];
    private GameObject[] angText = new GameObject[6];
    private GameObject[] posText = new GameObject[6];

    private float[] prevAngle = new float[SERVO_NUM];
    private float[] minAngle = new float[SERVO_NUM];
    private float[] maxAngle = new float[SERVO_NUM];
    float maxDeltaAngle = 60f; // 許容する最大の角度変化量（度単位）


    void Start()
    {
        for (int i = 0; i < joint.Length; i++){
            joint[i] = GameObject.Find("Joint_" + i.ToString());
            // slider[i] = GameObject.Find("Slider_" + i.ToString());
            // sliderVal[i] = slider[i].GetComponent<Slider>().value;
            // prevSliderVal[i] = sliderVal[i];
            posText[i] = GameObject.Find("Ref_" + i.ToString());
            angText[i] = GameObject.Find("Ang_" + i.ToString());
        }
                
        dim[0] = new Vector3(0f, 0f, 21f);    //point[0]とpoint[1]の配置
        dim[1] = new Vector3(0f, 0f, 102f);
        dim[2] = new Vector3(0f, 0f, 149f);
        dim[3] = new Vector3(0f, 0f, 145f);
        dim[4] = new Vector3(0f, 0f, 145f);
        dim[5] = new Vector3(0f, 0f, 107f);

        axis[0] = new Vector3(0f, 1f, 0f);
        axis[1] = new Vector3(1f, 0f, 0f);
        axis[2] = new Vector3(0f, 0f, 1f);
        axis[3] = new Vector3(1f, 0f, 0f);
        axis[4] = new Vector3(0f, 1f, 0f);
        axis[5] = new Vector3(1f, 0f, 0f);

        for (int i = 0; i < angle.Length; i++){
            angle[i] = 0f;
            prevAngle[i] = 0f;
            servoAngle[i] = 0f;
        }

        minAngle[0] = -60f;
        maxAngle[0] = 60f;
        minAngle[1] = -30f;
        maxAngle[1] = 90f;
        minAngle[2] = -135f;
        maxAngle[2] = 135f;
        minAngle[3] = -110f;
        maxAngle[3] = 110f;
        minAngle[4] = -110f;
        maxAngle[4] = 110f;
        minAngle[5] = -110f;
        maxAngle[5] = 110f;
        minAngle[6] = -80f;
        maxAngle[6] = 80f;


        angle[1] = 90f;
        angle[3] = -90f;

        posText[0].GetComponent<Text>().text = pos.x.ToString("f2");
        posText[1].GetComponent<Text>().text = pos.y.ToString("f2");
        posText[2].GetComponent<Text>().text = pos.z.ToString("f2");
        posText[3].GetComponent<Text>().text = rot.x.ToString("f2");
        posText[4].GetComponent<Text>().text = rot.y.ToString("f2");
        posText[5].GetComponent<Text>().text = rot.z.ToString("f2");
        
        for (int i = 0; i < joint.Length; i++){
            rotation[i] = Quaternion.AngleAxis(angle[i], axis[i]);
            joint[i].transform.localRotation = rotation[i];
            // prevSliderVal[i] = sliderVal[i];
            prevAngle[i] = angle[i];
            // posText[i].GetComponent<Text>().text = sliderVal[i].ToString("f2");
            angText[i].GetComponent<Text>().text = angle[i].ToString("f2");
        }


    }

    void Update()
    {
        /*for (int i = 0; i < joint.Length; i++){
            sliderVal[i] = slider[i].GetComponent<Slider>().value;
            }
        pos.x = sliderVal[0];
        pos.y = sliderVal[1];
        pos.z = sliderVal[2];
        rot.x = sliderVal[3];
        rot.y = sliderVal[4];
        rot.z = sliderVal[5];*/

        //左のコントローラの情報を取得
        pos = 1000 * OVRInput.GetLocalControllerPosition(OVRInput.Controller.LTouch);
        rot = OVRInput.GetLocalControllerRotation(OVRInput.Controller.LTouch).eulerAngles;
        //鏡写し
        pos.x = -pos.x;

        /*if (leftHand.IsPointerPoseValid){
            pos = leftHand.PointerPose.position;
            rot = leftHand.PointerPose.rotation.eulerAngles;
            //鏡写し
            pos.x = -pos.x;
            Debug.Log(pos.x + " " + pos.y + " " + pos.z + " " + rot.x + " " + rot.y + " " + rot.z);
        }*/

        if (body != null && body.BodyState != null){
            OVRPlugin.BodyState bodyState = body.BodyState.Value;
            OVRPlugin.BodyJointLocation[] jointLoacations = bodyState.JointLocations;

            //32: Left hand middle distal
            /*OVRPlugin.Posef leftHandPose = bodyState.JointLocations[32].Pose;
            pos = 1000 * new Vector3(leftHandPose.Position.x, leftHandPose.Position.y, leftHandPose.Position.z);
            rot = new Quaternion(leftHandPose.Orientation.x, leftHandPose.Orientation.y, leftHandPose.Orientation.z, leftHandPose.Orientation.w).eulerAngles;
            //鏡写し
            pos.x = -pos.x;*/

            //5: Chest
            OVRPlugin.Posef chestPose = jointLoacations[5].Pose;
            pos.x = pos.x - (1000 * chestPose.Position.x + 40f);
            pos.y = pos.y - 1000 * chestPose.Position.y + 180f;
            pos.z = pos.z - 1000 * chestPose.Position.z + 80f;
        }

        rot.x = FixPi(rot.x);
        rot.y = FixPi(rot.y);
        rot.z = FixPi(rot.z);

        
        //IK
        CalcIK();

        //Get vision angles
        var pose = getHMDPose.GetPose();
        var Euler = pose.Value.Rotation.eulerAngles;
        Euler.x = FixPi(Euler.x);
        Euler.y = FixPi(Euler.y);

        if (Euler.x > minAngle[6] && Euler.x < maxAngle[6]){
            //鏡写し
            angle[6] = Euler.y;
            prevAngle[6] = angle[6];
        } else {
            angle[6] = prevAngle[6];
        }
        /*if (Euler.y > minAngle[7] && Euler.y < maxAngle[7]){
            angle[7] = -Euler.y;
            prevAngle[7] = angle[7];
        } else {
            angle[7] = prevAngle[7];
        }*/

        for (int i = 0; i < SERVO_NUM; i++){
            servoAngle[i] = angle[i];
        }

        //符号整理
        servoAngle[0] = angle[0];
        //↓なぜ？
        servoAngle[1] = angle[1];
        servoAngle[2] = angle[2];
        servoAngle[3] = -angle[3];
        servoAngle[4] = angle[4];
        servoAngle[5] = -angle[5];
        servoAngle[6] = -angle[6];
        // servoAngle[7] = angle[7];

        //Send Angles
        TCP_Client.instance.SendMessageToServer("1:" + servoAngle[0].ToString("000.00") + ";2:" + servoAngle[1].ToString("000.00")
                                             + ";3:" + servoAngle[2].ToString("000.00") + ";4:" + servoAngle[3].ToString("000.00")
                                             + ";5:" + servoAngle[4].ToString("000.00") + ";6:" + servoAngle[5].ToString("000.00")
                                             + ";7:" + servoAngle[6].ToString("000.00") + ";\n"); 
    }

    void CalcIK()
    {
        int count = 0;
        
        //角度制限
        bool outOfLimit = false;

        for (int i = 0; i < 500; i++){
            count = i;
            ForwardKinematics();
            var err = CalcErr();
            float err_norm = (float)err.L2Norm();

            if (err_norm < 1E-3){
                for (int j = 0; j < joint.Length; j++){
                    if (angle[j] < minAngle[j] || angle[j] > maxAngle[j]){
                        outOfLimit = true;
                        break;
                    }
                }
                break;
            }
            var J = CalcJacobian();
            // var dAngle = lambda * J.PseudoInverse() * err;
            var dampingFactor = 0.01f;
            var JRegularized = J + dampingFactor * DenseMatrix.CreateIdentity(6);
            var dAngle = JRegularized.PseudoInverse() * err;
            
            for (int j = 0; j < joint.Length; j++){
                angle[j] += dAngle[j, 0] * Mathf.Rad2Deg;
                angle[j] = FixPi(angle[j]);
            }

            /*for (int j = 0; j < joint.Length; j++) {
                float deltaAngle = dAngle[j, 0] * Mathf.Rad2Deg; // ラジアンから度に変換
                if (Mathf.Abs(deltaAngle) > maxDeltaAngle) {
                    deltaAngle = Mathf.Sign(deltaAngle) * maxDeltaAngle; // 変化量を制限
                }
                angle[j] += deltaAngle;
                angle[j] = Mathf.Clamp(angle[j], minAngle[j], maxAngle[j]); // 範囲を超えないように制限
                angle[j] = FixPi(angle[j]); // 角度を正規化
            }*/
            

        }

        //収束せず
        if (count == 499 || outOfLimit){
            for (int i = 0; i < joint.Length; i++){
                // slider[i].GetComponent<Slider>().value = prevSliderVal[i];
                angle[i] = prevAngle[i];
            }
        }

        //収束した
        else{
            posText[0].GetComponent<Text>().text = pos.x.ToString("f2");
            posText[1].GetComponent<Text>().text = pos.y.ToString("f2");
            posText[2].GetComponent<Text>().text = pos.z.ToString("f2");
            posText[3].GetComponent<Text>().text = rot.x.ToString("f2");
            posText[4].GetComponent<Text>().text = rot.y.ToString("f2");
            posText[5].GetComponent<Text>().text = rot.z.ToString("f2");
            
            for (int i = 0; i < joint.Length; i++){
                rotation[i] = Quaternion.AngleAxis(angle[i], axis[i]);
                joint[i].transform.localRotation = rotation[i];
                // prevSliderVal[i] = sliderVal[i];
                prevAngle[i] = angle[i];
                // posText[i].GetComponent<Text>().text = sliderVal[i].ToString("f2");
                angText[i].GetComponent<Text>().text = angle[i].ToString("f2");
            }
        }
    }
    
    public static float FixPi(float Theta)
    {
        if (Theta > 180) {
            Theta -= Mathf.Floor((Theta + 180) / 360) * 360;
        }
        else if (Theta < -180) {
            Theta -= Mathf.Floor((Theta - 180) / 360) * 360; 
        }
        return Theta;
    }

    void ForwardKinematics()
    {
        point[0] = new Vector3(0f, 0f, 38f);
        wRotation[0] = Quaternion.AngleAxis(angle[0], axis[0]);
        for (int i = 1; i < joint.Length; i++){
            point[i] = wRotation[i - 1] * dim[i - 1] + point[i - 1];
            rotation[i] = Quaternion.AngleAxis(angle[i], axis[i]);
            wRotation[i] = wRotation[i - 1] * rotation[i];
        }
        point[joint.Length] = wRotation[joint.Length - 1] * dim[joint.Length - 1] + point[joint.Length - 1];
    }

    DenseMatrix CalcErr(){
        Vector3 perr = pos - point[joint.Length];
        Quaternion rerr = Quaternion.Euler(rot) * Quaternion.Inverse(wRotation[joint.Length - 1]);
        Vector3 rerrVal = new Vector3(rerr.eulerAngles.x, rerr.eulerAngles.y, rerr.eulerAngles.z);
        if (rerrVal.x > 180f){
            rerrVal.x -= 360f;
        }
        if (rerrVal.y > 180f){
            rerrVal.y -= 360f;
        }
        if (rerrVal.z > 180f){
            rerrVal.z -= 360f;
        }
        var err = DenseMatrix.OfArray(new float[,]{
            {perr.x},
            {perr.y},
            {perr.z},
            {rerrVal.x * Mathf.Deg2Rad},
            {rerrVal.y * Mathf.Deg2Rad},
            {rerrVal.z * Mathf.Deg2Rad},
        });
        return err;
    }
    DenseMatrix CalcJacobian(){
        Vector3 w0 = wRotation[0] * axis[0];
        Vector3 w1 = wRotation[1] * axis[1];
        Vector3 w2 = wRotation[2] * axis[2];
        Vector3 w3 = wRotation[3] * axis[3];
        Vector3 w4 = wRotation[4] * axis[4]; 
        Vector3 w5 = wRotation[5] * axis[5];
        Vector3 p0 = Vector3.Cross(w0, point[6] - point[0]);
        Vector3 p1 = Vector3.Cross(w1, point[6] - point[1]);
        Vector3 p2 = Vector3.Cross(w2, point[6] - point[2]);
        Vector3 p3 = Vector3.Cross(w3, point[6] - point[3]);
        Vector3 p4 = Vector3.Cross(w4, point[6] - point[4]);
        Vector3 p5 = Vector3.Cross(w5, point[6] - point[5]);

        var J = DenseMatrix.OfArray(new float[,]{
            {p0.x, p1.x, p2.x, p3.x, p4.x, p5.x},
            {p0.y, p1.y, p2.y, p3.y, p4.y, p5.y},
            {p0.z, p1.z, p2.z, p3.z, p4.z, p5.z},
            {w0.x, w1.x, w2.x, w3.x, w4.x, w5.x},
            {w0.y, w1.y, w2.y, w3.y, w4.y, w5.y},
            {w0.z, w1.z, w2.z, w3.z, w4.z, w5.z},
        });
        return J;
    }
}

