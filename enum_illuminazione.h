// Tipi di illuminazione
typedef enum
{
	NO_ILLUMINATION,
	PHONG,
	BLINN,
} IlluminationType;
// Tipi di materiali
typedef enum
{
	RED_PLASTIC,
	EMERALD,
	BRASS,
	SLATE,
	YELLOW,
	NO_MATERIAL
} MaterialType;
// Tipi di shader
typedef enum
{
	NO_SHADER,
	TEXTURE,
	PHONG_SHADING,
	INTERPOLATE_SHADING,
} ShaderType;
