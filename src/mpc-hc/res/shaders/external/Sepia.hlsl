/* --- Settings --- */

#define ColorTone float3(1.40, 1.10, 0.90) //[0.00 to 2.55, 0.00 to 2.55, 0.00 to 2.55] What color to tint the image
#define GreyPower  0.11                    //[0.00 to 1.00] How much desaturate the image before tinting it
#define SepiaPower 0.58                    //[0.00 to 1.00] How much to tint the image

/* ---  Defining Constants --- */
#define myTex2D(s,p) tex2D(s,p)

#ifndef s0
  sampler s0 : register(s0);
  #define s1 s0
//sampler s1 : register(s1);

  float4 p0 : register(c0);
  float4 p1 : register(c1);

//  #define width (p0[0])
//  #define height (p0[1])
//  #define counter (p0[2])
//  #define clock (p0[3])
//  #define px (p1[0]) //one_over_width 
//  #define py (p1[1]) //one_over_height

  #define px (p1.x) //one_over_width 
  #define py (p1.y) //one_over_height
  
  #define screen_size float2(p0.x,p0.y)

  #define pixel float2(px,py)

//#define pxy float2(p1.xy)

//#define PI acos(-1)
#endif


/* ---  Main code --- */

/*------------------------------------------------------------------------------
						SEPIA
------------------------------------------------------------------------------*/

float4 SepiaPass( float4 colorInput )
{
	float3 sepia = colorInput.rgb;
	
	// calculating amounts of input, grey and sepia colors to blend and combine
	float grey = dot(sepia, float3(0.2126, 0.7152, 0.0722));
	
	sepia *= ColorTone;
	
	float3 blend2 = (grey * GreyPower) + (colorInput.rgb / (GreyPower + 1));

	colorInput.rgb = lerp(blend2, sepia, SepiaPower);
	
	// returning the final color
	return colorInput;
}


/* --- Main --- */

float4 main(float2 tex : TEXCOORD0) : COLOR {
	float4 c0 = tex2D(s0, tex);

	c0 = SepiaPass(c0);
	return c0;
}