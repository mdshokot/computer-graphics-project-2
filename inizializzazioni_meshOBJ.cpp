#include "ShaderMaker.h"
#include "Gestione_VAO.h"
#include "geometria.h"
#include "Strutture.h"
#include "Materiali_Base.h"
#include "load_meshes_assimp.hpp"
#include "gestioneTexture.h"
const float M_PI = 3.14159265358979323846; // Definizione di M_PI


extern vector<MeshObj> Model3D;
extern vector<vector<MeshObj>> ScenaObj;

extern unsigned int programId, programId1, programIdr, programId_text, VAO_Text, VBO_Text;
Mesh Cubo, Piano, Piramide;
extern vector<Mesh> Scena;
extern int height, width;
extern ViewSetup SetupTelecamera;
extern PerspectiveSetup SetupProspettiva;
extern vector<Material> materials;
extern vector<Shader> shaders;
extern vector<Illumination> illuminations;
extern point_light light;
extern int texture;

void INIT_SHADER(void)

{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C.glsl";
	char* fragmentShader = (char*)"fragmentShader_C.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

	vertexShader = (char*)"vertexShader_CubeMap.glsl";
	fragmentShader = (char*)"fragmentShader_CubeMap.glsl";

	programId1 = ShaderMaker::createProgram(vertexShader, fragmentShader);
	// Generazione del program shader per la gestione del testo
	vertexShader = (char*)"VertexShader_Text.glsl";
	fragmentShader = (char*)"FragmentShader_Text.glsl";

	programId_text = ShaderMaker::createProgram(vertexShader, fragmentShader);

	vertexShader = (char*)"vertexShader_riflessione.glsl";
	fragmentShader = (char*)"fragmentShader_riflessione.glsl";

	programIdr = ShaderMaker::createProgram(vertexShader, fragmentShader);
}
// Clear the object
void clearObject(Mesh& oggetto) {
	oggetto.vertici.clear();
	oggetto.colori.clear();
	oggetto.indici.clear();
	oggetto.normali.clear();
	oggetto.texCoords.clear();
	oggetto.sceltaVS = 0;
}
// Crea l'oggetto dal file obj
void createObjectFromObj(vector<MeshObj> Model3D, string nome, int nmeshes, vec3 posizione, vec3 scala, float rad, vec3 rotation) {
	// Per ogni mesh dell'oggetto 3D creo un modello
	for (int i = 0; i < nmeshes; i++) {
		// Creo il VAO per la mesh
		crea_VAO_Vector_MeshObj(&Model3D[i]);
		Model3D[i].ModelM = mat4(1.0f);
		Model3D[i].ModelM = translate(Model3D[i].ModelM, posizione);
		Model3D[i].ModelM = scale(Model3D[i].ModelM, scala);
		if (rad != 0.0f) {
			Model3D[i].ModelM = rotate(Model3D[i].ModelM, radians(rad), rotation);
		}
		Model3D[i].nome = nome;
		Model3D[i].shader = ShaderType::INTERPOLATE_SHADING;
		Model3D[i].illumination = IlluminationType::BLINN;
		// Imposto il materiale
		vec3 ambiental = Model3D[i].materiale.ambient;
		vec3 difusivo = Model3D[i].materiale.diffuse;
		vec3 speculare = Model3D[i].materiale.specular;
		Model3D[i].materiale.ambient = ambiental;
		Model3D[i].materiale.diffuse = difusivo;
		Model3D[i].materiale.specular = speculare;
		Model3D[i].materiale.shininess = red_plastic_shininess;
		// Imposto lo shader
		Model3D[i].sceltaVS = 3;
	}
	// Inserisco il modello nella scena
	ScenaObj.push_back(Model3D);

	// Pulisco il Model3D per evitare duplicazioni
	Model3D.clear();
}

void INIT_VAO(void)
{
	Mesh oggetto;
	string Meshdir = "Meshes/", Imagedir = "Textures/", path, name;
	int nmeshes;
	bool obj;

	// Skybox
	crea_skybox(&oggetto, vec4(0.0f, 0.0f, 0.0f, 0.0f));
	crea_VAO_Vector(&oggetto);
	oggetto.Model = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
	oggetto.Model = scale(oggetto.Model, vec3(1.0f, 1.0f, 1.0f));
	oggetto.nome = "Skybox";
	oggetto.shader = ShaderType::NO_SHADER;
	oggetto.material = MaterialType::NO_MATERIAL;
	oggetto.illumination = IlluminationType::NO_ILLUMINATION;
	Scena.push_back(oggetto);
	clearObject(oggetto);

	Model3D.clear();
	// CharizardMegaY
	name = "CharizardMegaY.obj";
	path = Meshdir + name;
	obj = loadAssImp(path.c_str(), Model3D);
	nmeshes = Model3D.size();
	createObjectFromObj(Model3D, "CharizardMegaY ", nmeshes, vec3(0.0, -10.0, 100.0), vec3(5, 5, 5), 0.0, vec3(1.0, 1.0, 1.0));
	Model3D.clear();

	// Sfera 3
	float raggio = 20.0f;  // Raggio del cerchio
	int numeroSfere = 7;   // Numero di sfere
	vec3 centroCerchio = vec3(0.0f, -20.0f, 100.0f);  // Centro del cerchio

	name = "dragon_ball.jpg";
	path = Imagedir + name;

	for (int i = 0; i < numeroSfere; ++i) {
		float angolo = i * 2.0f * M_PI / numeroSfere;
		float x = centroCerchio.x + raggio * cos(angolo);
		float z = centroCerchio.z + raggio * sin(angolo);

		Mesh oggetto;
		crea_sfera(&oggetto, vec4(1.0, 0.0, 1.0, 1.0));
		crea_VAO_Vector(&oggetto);
		oggetto.Model = mat4(1.0);
		oggetto.Model = translate(oggetto.Model, vec3(x, centroCerchio.y, z));
		oggetto.Model = scale(oggetto.Model, vec3(2.0, 2.0, 2.0)); 
		oggetto.nome = "Sfera " + to_string(i);
		oggetto.shader = ShaderType::TEXTURE;
		oggetto.illumination = IlluminationType::BLINN;
		oggetto.material = MaterialType::YELLOW;
		oggetto.sceltaVS = loadTexture(path.c_str(), 0);
		Scena.push_back(oggetto);
		clearObject(oggetto);
	}

}

void INIT_VAO_Text(void)
{
	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO_Text);
	glGenBuffers(1, &VBO_Text);
	glBindVertexArray(VAO_Text);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void INIT_CAMERA_PROJECTION(void)
{
	// Imposto la telecamera
	SetupTelecamera.position = glm::vec4(0.0, 0.5, 150.0, 0.0);
	SetupTelecamera.target = glm::vec4(0.0, 0.0, 0.0, 0.0);
	SetupTelecamera.direction = SetupTelecamera.target - SetupTelecamera.position;
	SetupTelecamera.upVector = glm::vec4(0.0, 1.0, 0.0, 0.0);

	// Imposto la proiezione prospettica
	SetupProspettiva.aspect = (GLfloat)width / (GLfloat)height;
	SetupProspettiva.fovY = 45.0f;
	SetupProspettiva.far_plane = 2000.0f;
	SetupProspettiva.near_plane = 0.1f;
}

void INIT_Illuminazione()
{
	light.position = { 1.0, 1.0, 1.0 };
	light.color = { 1.0, 1.0, 1.0 };
	light.power = 1.0f;

	// Setup dei materiali
	// Materials setup
	materials.resize(6);
	materials[MaterialType::RED_PLASTIC].name = "Red Plastic";
	materials[MaterialType::RED_PLASTIC].ambient = red_plastic_ambient;
	materials[MaterialType::RED_PLASTIC].diffuse = red_plastic_diffuse;
	materials[MaterialType::RED_PLASTIC].specular = red_plastic_specular;
	materials[MaterialType::RED_PLASTIC].shininess = red_plastic_shininess;

	materials[MaterialType::EMERALD].name = "Emerald";
	materials[MaterialType::EMERALD].ambient = emerald_ambient;
	materials[MaterialType::EMERALD].diffuse = emerald_diffuse;
	materials[MaterialType::EMERALD].specular = emerald_specular;
	materials[MaterialType::EMERALD].shininess = emerald_shininess;

	materials[MaterialType::BRASS].name = "Brass";
	materials[MaterialType::BRASS].ambient = brass_ambient;
	materials[MaterialType::BRASS].diffuse = brass_diffuse;
	materials[MaterialType::BRASS].specular = brass_specular;
	materials[MaterialType::BRASS].shininess = brass_shininess;

	materials[MaterialType::SLATE].name = "Slate";
	materials[MaterialType::SLATE].ambient = slate_ambient;
	materials[MaterialType::SLATE].diffuse = slate_diffuse;
	materials[MaterialType::SLATE].specular = slate_specular;
	materials[MaterialType::SLATE].shininess = slate_shininess;

	materials[MaterialType::YELLOW].name = "Yellow";
	materials[MaterialType::YELLOW].ambient = yellow_ambient;
	materials[MaterialType::YELLOW].diffuse = yellow_diffuse;
	materials[MaterialType::YELLOW].specular = yellow_specular;
	materials[MaterialType::YELLOW].shininess = yellow_shininess;

	materials[MaterialType::NO_MATERIAL].name = "NO_MATERIAL";
	materials[MaterialType::NO_MATERIAL].ambient = glm::vec3(1, 1, 1);
	materials[MaterialType::NO_MATERIAL].diffuse = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].specular = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].shininess = 1.f;

	// Setup degli shader
	shaders.resize(4);
	shaders[ShaderType::NO_SHADER].name = "NO SHADER";
	shaders[ShaderType::TEXTURE].name = "TEXTURE";
	shaders[ShaderType::PHONG_SHADING].name = "PHONG SHADING";
	shaders[ShaderType::INTERPOLATE_SHADING].name = "INTERPOLATE SHADING";

	illuminations.resize(3);
	illuminations[IlluminationType::NO_ILLUMINATION].name = "NONE";
	illuminations[IlluminationType::PHONG].name = "PHONG";
	illuminations[IlluminationType::BLINN].name = "BLINN";
}