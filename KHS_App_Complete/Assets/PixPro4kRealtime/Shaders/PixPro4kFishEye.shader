// Upgrade NOTE: replaced 'mul(UNITY_MATRIX_MVP,*)' with 'UnityObjectToClipPos(*)'

// Copyright (c) 2016 Yasuhide Okamoto
// Released under the MIT license
// http://opensource.org/licenses/mit-license.php

Shader "PIXPRO/PixPro4kFishEye"
{
    Properties
    {
        _MainTex("Texture", 2D) = "white" {}
        _KAngle("k angle", float) = 150.0
        _RLength("長辺に対する魚眼の半径の割合", float) = 0.25
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            // Make fog work
            #pragma multi_compile_fog
            
            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                UNITY_FOG_COORDS(1)
                float4 vertex : SV_POSITION;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            float _KAngle;
            float _RLength;

            v2f vert(appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex); // Transform vertex position
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);     // Transform UV with scale/offset
                UNITY_TRANSFER_FOG(o, o.vertex);          // Transfer fog data
                return o;
            }
            
            fixed4 frag(v2f i) : SV_Target
            {
                // Convert UV coordinates to polar coordinates (phi)
                float phi = i.uv[0] * 2.0 * UNITY_PI;

                // Radius calculation
                float r = (1.0 - i.uv[1]) * 360.0 * _RLength / _KAngle;

                // Generate fisheye UV
                float2 fisheye_uv = float2 (0.5, 0.5) + float2(cos(phi), -sin(phi)) * r;

                // Clockwise 90-degree rotation matrix
                float2x2 rotationMatrix = float2x2(0, -1, 1, 0);

                // Apply rotation: move UV to origin, rotate, then move back
                fisheye_uv = mul(rotationMatrix, fisheye_uv - 0.5) + 0.5;

                // Clip UV to ensure it is within the texture bounds
                float in_tex = step(0.0, fisheye_uv.x) * step(fisheye_uv.x, 1.0) * step(0.0, fisheye_uv.y) * step(fisheye_uv.y, 1.0);

                // Sample texture using the transformed UV
                float4 col = tex2D(_MainTex, fisheye_uv) * in_tex;

                // Apply fog to the final color
                UNITY_APPLY_FOG(i.fogCoord, col);

                return col;
            }
            ENDCG
        }
    }
}