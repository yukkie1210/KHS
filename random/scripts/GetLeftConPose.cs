using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR;

public class GetLeftConPose : MonoBehaviour
{
    private InputDevice leftCon;

    void Start()
    {
        // 左コントローラを取得
        var inputDevices = new List<InputDevice>();
        InputDevices.GetDevicesWithCharacteristics(InputDeviceCharacteristics.Left | InputDeviceCharacteristics.Controller, inputDevices);

        if (inputDevices.Count > 0)
        {
            leftCon = inputDevices[0];
            Debug.Log("Left controller found: " + leftCon.name);
        }
        else
        {
            Debug.LogWarning("Left controller not found.");
        }
    }

    void Update()
    {
        // 左コントローラの座標を取得
        if (leftCon.isValid)
        {
            if (leftCon.TryGetFeatureValue(CommonUsages.devicePosition, out Vector3 position))
            {
                Debug.Log("Left controller position: " + position);
            }
            if (leftCon.TryGetFeatureValue(CommonUsages.deviceRotation, out Quaternion rotation))
            {
                Debug.Log("Left controller rotation: " + rotation);
            }
        }
        else
        {
            Debug.LogWarning("Left controller is not valid.");
        }
    }
}