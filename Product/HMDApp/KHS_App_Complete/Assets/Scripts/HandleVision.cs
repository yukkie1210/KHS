using UnityEngine;

public class HandleVision : MonoBehaviour
{
    public Transform centerEyeAnchor;
    // public OVRHand leftHand;
    // public float pinchThreshold = 0.7f; // ピンチの強さの閾値

    private bool isVisible = true;
    private MeshRenderer objectRenderer;

    void Start()
    {
        // Renderer を取得
        objectRenderer = GetComponent<MeshRenderer>();

        if (objectRenderer == null)
        {
            Debug.LogError("Renderer component is missing on this GameObject.");
        }
    }

    void Update()
    {
        if (centerEyeAnchor != null)
        {
            transform.position = centerEyeAnchor.position;
        }

        if (OVRInput.GetDown(OVRInput.Button.PrimaryIndexTrigger))
        {
            isVisible = !isVisible;
            UpdateVisibility();
        }
        
        /*if (leftHand != null)
        {
            // 親指と人差し指のピンチ強度を取得
            float pinchStrength = leftHand.GetFingerPinchStrength(OVRHand.HandFinger.Index);

            // 閾値を超えた場合、ピンチと判定
            if (pinchStrength >= pinchThreshold || OVRInput.GetDown(OVRInput.Button.PrimaryIndexTrigger))
            {
                isVisible = !isVisible;
                UpdateVisibility();
            }
        }*/
    }

    private void UpdateVisibility()
    {
        if (objectRenderer != null)
        {
            objectRenderer.enabled = isVisible;
        }
    }
}