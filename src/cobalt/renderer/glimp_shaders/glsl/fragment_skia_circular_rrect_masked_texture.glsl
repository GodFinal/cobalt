#version 100
precision mediump float;
uniform sampler2D uSampler0_Stage0;
uniform vec4 uinnerRect_Stage1;
uniform float uradiusPlusHalf_Stage1;
uniform float uRTHeight;
varying vec4 vColor;
varying vec2 vMatrixCoord_Stage0;
void main() 
{
	vec4 fragCoordYDown = vec4(gl_FragCoord.x, uRTHeight - gl_FragCoord.y, 1.0, 1.0);
	vec4 output_Stage0;
	{
		// Stage 0: Texture
		output_Stage0 = (vColor * texture2D(uSampler0_Stage0, vMatrixCoord_Stage0));
	}
	vec4 output_Stage1;
	{
		// Stage 1: CircularRRect
		vec2 dxy0 = uinnerRect_Stage1.xy - fragCoordYDown.xy;
		vec2 dxy1 = fragCoordYDown.xy - uinnerRect_Stage1.zw;
		vec2 dxy = max(max(dxy0, dxy1), 0.0);
		float alpha = clamp(uradiusPlusHalf_Stage1 - length(dxy), 0.0, 1.0);
		output_Stage1 = (output_Stage0 * alpha);
	}
	gl_FragColor = output_Stage1;
}
