using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR;

public class GetHMDPose : MonoBehaviour
{
    private InputDevice hmdDevice; // HMDデバイスを格納

    void Start()
    {
        // HMDデバイスを取得
        var inputDevices = new List<InputDevice>();
        InputDevices.GetDevicesWithCharacteristics(InputDeviceCharacteristics.HeadMounted, inputDevices);

        if (inputDevices.Count > 0)
        {
            hmdDevice = inputDevices[0];
            Debug.Log("HMD found: " + hmdDevice.name);
        }
        else
        {
            Debug.LogWarning("HMD not found.");
        }
    }

    void Update()
    {
        // HMDの姿勢（位置と回転）を取得
        if (hmdDevice.isValid)
        {
            // 位置を取得
            if (hmdDevice.TryGetFeatureValue(CommonUsages.devicePosition, out Vector3 position))
            {
                Debug.Log("HMD Position: " + position);
            }

            // 回転を取得
            if (hmdDevice.TryGetFeatureValue(CommonUsages.deviceRotation, out Quaternion rotation))
            {
                Debug.Log("HMD Rotation: " + rotation);
            }
        }
        else
        {
            Debug.LogWarning("HMD device is not valid.");
        }
    }
    public bool GetPose(out Vector3 position, out Quaternion rotation)
    {
        position = Vector3.zero;
        rotation = Quaternion.identity;
        if (hmdDevice.isValid)
        {
            hmdDevice.TryGetFeatureValue(CommonUsages.devicePosition, out position);
            hmdDevice.TryGetFeatureValue(CommonUsages.deviceRotation, out rotation);
            return true;
        }
        return false;
    }
}