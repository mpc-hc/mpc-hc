#define LUT3D_ENABLED (_LUT3D_ENABLED_VALUE_)

sampler image : register(s0);

sampler ditherMatrix : register(s1);
float2 ditherMatrixCoordScale : register(c0);

// Maximum quantized integer value
static const float QUANTIZATION = _QUANTIZATION_VALUE_;

#if LUT3D_ENABLED
sampler lut3D : register(s2);
static const float LUT3D_SIZE = _LUT3D_SIZE_VALUE_;

// 3D LUT texture coordinate scale and offset required for correct linear interpolation
static const float LUT3D_SCALE = (LUT3D_SIZE - 1.0f) / LUT3D_SIZE;
static const float LUT3D_OFFSET = 1.0f / (2.0f * LUT3D_SIZE);
#endif

float4 main(float2 imageCoord : TEXCOORD0) : COLOR {
	float4 pixel = tex2D(image, imageCoord);

#if LUT3D_ENABLED
	pixel = tex3D(lut3D, pixel.rgb * LUT3D_SCALE + LUT3D_OFFSET);
#endif

	float4 ditherValue = tex2D(ditherMatrix, imageCoord * ditherMatrixCoordScale);
	return floor(pixel * QUANTIZATION + ditherValue) / QUANTIZATION;
}
