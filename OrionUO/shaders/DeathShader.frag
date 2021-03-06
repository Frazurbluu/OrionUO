uniform sampler2D usedTexture;

void main(void)
{
	vec4 textureColor = texture2D(usedTexture, gl_TexCoord[0].xy);

	float red = textureColor.r * 0.85;

	if (red < 0.01)
	{
		red = textureColor.g * 0.85;

		if (red < 0.01)
		{
			red = textureColor.b * 0.85;

			if (red < 0.01)
				red = 0.1;
		}
	}

	gl_FragColor = vec4(red, red, red, textureColor.a) * gl_Color;
}
