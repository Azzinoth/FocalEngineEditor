in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ haloBluredTexture;
@Texture@ haloMaskTexture;

out vec4 out_Color;

void main(void)
{
	if (texture(haloMaskTexture, textureCoords).x < 0.0001 && texture(haloMaskTexture, textureCoords).y < 0.0001 && texture(haloMaskTexture, textureCoords).z < 0.0001)
	{
		out_Color = texture(sceneTexture, textureCoords) + texture(haloBluredTexture, textureCoords);
	}
	else
	{
		out_Color = texture(sceneTexture, textureCoords);
	}
}