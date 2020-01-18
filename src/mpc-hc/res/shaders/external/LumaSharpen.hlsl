/* --- Settings --- */

// -- Sharpening --
#define sharp_strength 0.65   //[0.10 to 3.00] Strength of the sharpening
#define sharp_clamp    0.035  //[0.000 to 1.000] Limits maximum amount of sharpening a pixel recieves - Default is 0.035

// -- Advanced sharpening settings --
#define pattern 2        //[1|2|3|4] Choose a sample pattern. 1 = Fast, 2 = Normal, 3 = Wider, 4 = Pyramid shaped.
#define offset_bias 1.0  //[0.0 to 6.0] Offset bias adjusts the radius of the sampling pattern.
                         //I designed the pattern for offset_bias 1.0, but feel free to experiment.

// -- Debug sharpening settings --
#define show_sharpen 0   //[0 or 1] Visualize the strength of the sharpen (multiplied by 4 to see it better)


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

/*
   _____________________

     LumaSharpen 1.4.1
   _____________________

  by Christian Cann Schuldt Jensen ~ CeeJay.dk

  It blurs the original pixel with the surrounding pixels and then subtracts this blur to sharpen the image.
  It does this in luma to avoid color artifacts and allows limiting the maximum sharpning to avoid or lessen halo artifacts.

  This is similar to using Unsharp Mask in Photoshop.

  Compiles with 3.0
*/

   /*-----------------------------------------------------------.
  /                      Developer settings                     /
  '-----------------------------------------------------------*/
#define CoefLuma float3(0.2126, 0.7152, 0.0722)      // BT.709 & sRBG luma coefficient (Monitors and HD Television)
//#define CoefLuma float3(0.299, 0.587, 0.114)       // BT.601 luma coefficient (SD Television)
//#define CoefLuma float3(1.0/3.0, 1.0/3.0, 1.0/3.0) // Equal weight coefficient

   /*-----------------------------------------------------------.
  /                          Main code                          /
  '-----------------------------------------------------------*/

float4 LumaSharpenPass(float4 inputcolor, float2 tex )
{
  // -- Get the original pixel --
  float3 ori = myTex2D(s0, tex).rgb;       // ori = original pixel

  // -- Combining the strength and luma multipliers --
  float3 sharp_strength_luma = (CoefLuma * sharp_strength); //I'll be combining even more multipliers with it later on

   /*-----------------------------------------------------------.
  /                       Sampling patterns                     /
  '-----------------------------------------------------------*/
  //   [ NW,   , NE ] Each texture lookup (except ori)
  //   [   ,ori,    ] samples 4 pixels
  //   [ SW,   , SE ]

  // -- Pattern 1 -- A (fast) 7 tap gaussian using only 2+1 texture fetches.
  #if pattern == 1

	// -- Gaussian filter --
	//   [ 1/9, 2/9,    ]     [ 1 , 2 ,   ]
	//   [ 2/9, 8/9, 2/9]  =  [ 2 , 8 , 2 ]
 	//   [    , 2/9, 1/9]     [   , 2 , 1 ]

    float3 blur_ori = myTex2D(s0, tex + (float2(px,py) / 3.0) * offset_bias).rgb;  // North West
    blur_ori += myTex2D(s0, tex + (float2(-px,-py) / 3.0) * offset_bias).rgb; // South East

    //blur_ori += myTex2D(s0, tex + float2(px,py) / 3.0 * offset_bias); // North East
    //blur_ori += myTex2D(s0, tex + float2(-px,-py) / 3.0 * offset_bias); // South West

    blur_ori /= 2;  //Divide by the number of texture fetches

    sharp_strength_luma *= 1.5; // Adjust strength to aproximate the strength of pattern 2

  #endif

  // -- Pattern 2 -- A 9 tap gaussian using 4+1 texture fetches.
  #if pattern == 2

	// -- Gaussian filter --
	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]
	//   [ .50,   1, .50]  =  [ 2 , 4 , 2 ]
 	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]


    float3 blur_ori = myTex2D(s0, tex + float2(px,-py) * 0.5 * offset_bias).rgb; // South East
    blur_ori += myTex2D(s0, tex + float2(-px,-py) * 0.5 * offset_bias).rgb;  // South West
    blur_ori += myTex2D(s0, tex + float2(px,py) * 0.5 * offset_bias).rgb; // North East
    blur_ori += myTex2D(s0, tex + float2(-px,py) * 0.5 * offset_bias).rgb; // North West

    blur_ori *= 0.25;  // ( /= 4) Divide by the number of texture fetches

  #endif

  // -- Pattern 3 -- An experimental 17 tap gaussian using 4+1 texture fetches.
  #if pattern == 3

	// -- Gaussian filter --
	//   [   , 4 , 6 ,   ,   ]
	//   [   ,16 ,24 ,16 , 4 ]
	//   [ 6 ,24 ,   ,24 , 6 ]
	//   [ 4 ,16 ,24 ,16 ,   ]
	//   [   ,   , 6 , 4 ,   ]

    float3 blur_ori = myTex2D(s0, tex + float2(0.4*px,-1.2*py)* offset_bias).rgb;  // South South East
    blur_ori += myTex2D(s0, tex + float2(-1.2*px,-0.4*py) * offset_bias).rgb; // West South West
    blur_ori += myTex2D(s0, tex + float2(1.2*px,0.4*py) * offset_bias).rgb; // East North East
    blur_ori += myTex2D(s0, tex + float2(-0.4*px,1.2*py) * offset_bias).rgb; // North North West

    blur_ori *= 0.25;  // ( /= 4) Divide by the number of texture fetches

    sharp_strength_luma *= 0.51;
  #endif

  // -- Pattern 4 -- A 9 tap high pass (pyramid filter) using 4+1 texture fetches.
  #if pattern == 4

	// -- Gaussian filter --
	//   [ .50, .50, .50]     [ 1 , 1 , 1 ]
	//   [ .50,    , .50]  =  [ 1 ,   , 1 ]
 	//   [ .50, .50, .50]     [ 1 , 1 , 1 ]

    float3 blur_ori = myTex2D(s0, tex + float2(0.5 * px,-py * offset_bias)).rgb;  // South South East
    blur_ori += myTex2D(s0, tex + float2(offset_bias * -px,0.5 * -py)).rgb; // West South West
    blur_ori += myTex2D(s0, tex + float2(offset_bias * px,0.5 * py)).rgb; // East North East
    blur_ori += myTex2D(s0, tex + float2(0.5 * -px,py * offset_bias)).rgb; // North North West

    //blur_ori += (2 * ori); // Probably not needed. Only serves to lessen the effect.

    blur_ori /= 4.0;  //Divide by the number of texture fetches

    sharp_strength_luma *= 0.666; // Adjust strength to aproximate the strength of pattern 2
  #endif

  // -- Pattern 8 -- A (slower) 9 tap gaussian using 9 texture fetches.
  #if pattern == 8

	// -- Gaussian filter --
	//   [ 1 , 2 , 1 ]
	//   [ 2 , 4 , 2 ]
 	//   [ 1 , 2 , 1 ]

    half3 blur_ori = myTex2D(s0, tex + float2(-px,py) * offset_bias).rgb; // North West
    blur_ori += myTex2D(s0, tex + float2(px,-py) * offset_bias).rgb;     // South East
    blur_ori += myTex2D(s0, tex + float2(-px,-py)  * offset_bias).rgb;  // South West
    blur_ori += myTex2D(s0, tex + float2(px,py) * offset_bias).rgb;    // North East

    half3 blur_ori2 = myTex2D(s0, tex + float2(0,py) * offset_bias).rgb; // North
    blur_ori2 += myTex2D(s0, tex + float2(0,-py) * offset_bias).rgb;    // South
    blur_ori2 += myTex2D(s0, tex + float2(-px,0) * offset_bias).rgb;   // West
    blur_ori2 += myTex2D(s0, tex + float2(px,0) * offset_bias).rgb;   // East
    blur_ori2 *= 2.0;

    blur_ori += blur_ori2;
    blur_ori += (ori * 4); // Probably not needed. Only serves to lessen the effect.

    // dot()s with gaussian strengths here?

    blur_ori /= 16.0;  //Divide by the number of texture fetches

    //sharp_strength_luma *= 0.75; // Adjust strength to aproximate the strength of pattern 2
  #endif

  // -- Pattern 9 -- A (slower) 9 tap high pass using 9 texture fetches.
  #if pattern == 9

	// -- Gaussian filter --
	//   [ 1 , 1 , 1 ]
	//   [ 1 , 1 , 1 ]
 	//   [ 1 , 1 , 1 ]

    float3 blur_ori = myTex2D(s0, tex + float2(-px,py) * offset_bias).rgb; // North West
    blur_ori += myTex2D(s0, tex + float2(px,-py) * offset_bias).rgb;     // South East
    blur_ori += myTex2D(s0, tex + float2(-px,-py)  * offset_bias).rgb;  // South West
    blur_ori += myTex2D(s0, tex + float2(px,py) * offset_bias).rgb;    // North East

    blur_ori += ori.rgb; // Probably not needed. Only serves to lessen the effect.

    blur_ori += myTex2D(s0, tex + float2(0,py) * offset_bias).rgb;    // North
    blur_ori += myTex2D(s0, tex + float2(0,-py) * offset_bias).rgb;  // South
    blur_ori += myTex2D(s0, tex + float2(-px,0) * offset_bias).rgb; // West
    blur_ori += myTex2D(s0, tex + float2(px,0) * offset_bias).rgb; // East

    blur_ori /= 9;  //Divide by the number of texture fetches

    //sharp_strength_luma *= (8.0/9.0); // Adjust strength to aproximate the strength of pattern 2
  #endif


   /*-----------------------------------------------------------.
  /                            Sharpen                          /
  '-----------------------------------------------------------*/

  // -- Calculate the sharpening --
  float3 sharp = ori - blur_ori;  //Subtracting the blurred image from the original image

  #if 0 //New experimental limiter .. not yet finished
    float sharp_luma = dot(sharp, sharp_strength_luma); //Calculate the luma
    sharp_luma = (abs(sharp_luma)*8.0) * exp(1.0-(abs(sharp_luma)*8.0)) * sign(sharp_luma) / 16.0; //I should probably move the strength modifier here

  #elif 0 //SweetFX 1.4 code
    // -- Adjust strength of the sharpening --
    float sharp_luma = dot(sharp, sharp_strength_luma); //Calculate the luma and adjust the strength

    // -- Clamping the maximum amount of sharpening to prevent halo artifacts --
    sharp_luma = clamp(sharp_luma, -sharp_clamp, sharp_clamp);  //TODO Try a curve function instead of a clamp
  
  #else //SweetFX 1.5.1 code
    // -- Adjust strength of the sharpening and clamp it--
    float4 sharp_strength_luma_clamp = float4(sharp_strength_luma * (0.5 / sharp_clamp),0.5); //Roll part of the clamp into the dot

    //sharp_luma = saturate((0.5 / sharp_clamp) * sharp_luma + 0.5); //scale up and clamp
    float sharp_luma = saturate(dot(float4(sharp,1.0), sharp_strength_luma_clamp)); //Calculate the luma, adjust the strength, scale up and clamp
    sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp; //scale down
  #endif

  // -- Combining the values to get the final sharpened pixel	--
  //float4 done = ori + sharp_luma;    // Add the sharpening to the original.
  inputcolor.rgb = inputcolor.rgb + sharp_luma;    // Add the sharpening to the input color.

   /*-----------------------------------------------------------.
  /                     Returning the output                    /
  '-----------------------------------------------------------*/
  #if show_sharpen == 1
    //inputcolor.rgb = abs(sharp * 4.0);
    inputcolor.rgb = saturate(0.5 + (sharp_luma * 4)).rrr;
  #endif

  return saturate(inputcolor);

}

/* --- Main --- */

float4 main(float2 tex : TEXCOORD0) : COLOR {
	float4 c0 = tex2D(s0, tex);

	c0 = LumaSharpenPass(c0, tex);
	return c0;
}
