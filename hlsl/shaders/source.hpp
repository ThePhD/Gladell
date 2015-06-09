#pragma once

#include "../../string.hpp"

namespace gld { namespace hlsl { namespace shaders { 

	namespace fluff {
		string_view pre_processing = R"(
#define a b
#define c( d ) e
#define f(g) h
#define i(j)k
#include "file.jk"
#pragma pack_matrix ( row_major )
#ifdef MEOW
invalid invalid invalid 4kjfwh
#else
#define MEOW_IS_DEFINED
#endif

#if defined(ARF) && ARF < 245
#define ARF_LESS_THAN_245
#elif defined(BARK) && BARK > 245
#define BARK_GREATER_THAN_245
#endif // ARF/BARK

#define song do\
re\
mi\
fa\
so\
la\
ti\
dooo!

)";
	}

	namespace sm40_level_93 {
		
	string_view nymph_batch = R"(
#define vec2 float2
#define vec3 float3
#define vec4 float4

float4x4 ViewProjection;

cbuffer Batch {
	float2 Texture0Size;
	float2 Texture0PixelSize;
}

texture Tex0;
sampler Texture0;

void NymphVertex( inout float4 position : SV_Position,
	inout float2 tex : TEXCOORD0,
	inout float4 color : COLOR0 ) {


							position = mul( position, ViewProjection );
}

float4 NymphPixel( float4 position : SV_Position,
	float2 tex : TEXCOORD0,
	float4 color : COLOR0 ) : SV_Target0{

							return tex2D( Texture0, tex ) * color;
}

float4 NymphTexturePixel( float4 position : SV_Position,
	float2 tex : TEXCOORD0,
	float4 color : COLOR0 ) : SV_Target0{

							float4 outputcolor = outputcolor = tex2D( Texture0, tex.xy );
	outputcolor.a = color.a;
	if ( any( outputcolor.rgb ) ) {
		outputcolor.rgb *= color.rgb;
	}
	else {
		outputcolor.rgb = color.rgb;
	}

							return outputcolor;
}

float4 NymphColorPixel( float4 position : SV_Position,
	float2 tex : TEXCOORD0,
	float4 color : COLOR0 ) : SV_Target0{

							return color;
}

technique NymphBatch {
	pass Regular {
		VertexShader = compile vs_5_0 NymphVertex( );
		PixelShader = compile ps_5_0 NymphPixel( );
	}
	pass Color {
		VertexShader = compile vs_5_0 NymphVertex( );
		PixelShader = compile ps_5_0 NymphColorPixel( );
	}
	pass TextureOnly {
		VertexShader = compile vs_5_0 NymphVertex( );
		PixelShader = compile ps_5_0 NymphTexturePixel( );
	}
}
)";

}}}}
