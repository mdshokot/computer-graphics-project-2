#include <iostream>

#include "ShaderMaker.h"
#include "Lib.h"
#include "geometria.h"

#include "Gestione_VAO.h"
#include "GestioneTesto.h"
#include "inizializzazioni..h"
#include "GestioneInterazioni.h"
#include "GestioneTelecamera.h"
#include "load_meshes_assimp.hpp"
#include "gestioneTexture.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
int last_mouse_pos_Y;
int last_mouse_pos_X;
bool moving_trackball = 0;
float angolo;
// Impostazione larghezza ed altezza della finestra sullo schermo
int width = 1024;
int height = 800;
// Definizione stringhe da visualizzare sulla finestra
string stringa_asse;
string Operazione;
vector<Material> materials;
vector<Shader> shaders;
vector<Illumination> illuminations;
LightShaderUniform light_unif = {};
vector<MeshObj> Model3D;
vector<vector<MeshObj>> ScenaObj;

int w_up = width;
int h_up = height;
mat4 Projection_text;
mat4 rotation_matrix = mat4(1.0);

unsigned int programId, programId1, programIdr, programId_text, MatrixProj, MatrixProj_txt, MatModel, MatView, locSceltaVs, locIllumination, loctime, MatViewS, MatrixProjS;
unsigned int loc_view_pos, VAO_Text, VBO_Text, loc_texture;
int idTex, texture, texture1, texture2, cubemapTexture;
unsigned int MatModelR, MatViewR, MatrixProjR, loc_view_posR, loc_cubemapR;
float raggio_sfera = 2.5;
vec3 asse = vec3(0.0, 1.0, 0.0);
int selected_obj = 0;
float cameraSpeed = 1.0;
vector<Mesh> Scena, Snowman;
point_light light;

// Definzione SetupTelecamera e SetupProspettiva

ViewSetup SetupTelecamera;
PerspectiveSetup SetupProspettiva;

// Varibili per la gestione della direzione della telecamera tramite mouse
bool firstMouse = true;
float lastX = (float)width / 2;
float lastY = (float)height / 2;
float Theta = -90.0f;
float Phi = 0.0f;

string SkyboxDir = "SkyBox/";

mat4 Projection, Model, View;
void inizilizzaCubemap()
{
	vector<std::string> faces{
			SkyboxDir + "right.jpg",
			SkyboxDir + "left.jpg",
			SkyboxDir + "top.jpg",
			SkyboxDir + "bottom.jpg",
			SkyboxDir + "front.jpg",
			SkyboxDir + "back.jpg" };
	cubemapTexture = loadCubemap(faces, 0);
}
// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
//
void resize(int w, int h)
{
	// Imposto la matrice di Proiezione per il rendering del testo
	Projection_text = ortho(0.0f, (float)width, 0.0f, (float)height);

	// Imposto la matrice di proiezione per la scena da disegnare
	Projection = perspective(radians(SetupProspettiva.fovY), SetupProspettiva.aspect, SetupProspettiva.near_plane, SetupProspettiva.far_plane);

	float AspectRatio_mondo = (float)(width) / (float)(height); // Rapporto larghezza altezza di tutto ci� che � nel mondo
	// Se l'aspect ratio del mondo � diversa da quella della finestra devo mappare in modo diverso
	// per evitare distorsioni del disegno
	if (AspectRatio_mondo > w / h) // Se ridimensioniamo la larghezza della Viewport
	{
		glViewport(0, 0, w, w / AspectRatio_mondo);
		w_up = (float)w;
		h_up = w / AspectRatio_mondo;
	}
	else
	{ // Se ridimensioniamo la larghezza della viewport oppure se l'aspect ratio tra la finestra del mondo
		// e la finestra sullo schermo sono uguali
		glViewport(0, 0, h * AspectRatio_mondo, h);
		w_up = h * AspectRatio_mondo;
		h_up = (float)h;
	}
}
// Crea il menu per la scelta dei materiali, shader e illuminazione
void main_menu_func(int option)
{
	glutPostRedisplay();
}

void material_menu_function(int option)
{
	if (selected_obj > -1) {
		Scena[selected_obj].material = (MaterialType)option;
	}
	glutPostRedisplay();
}

void shader_menu_function(int option)
{
	if (selected_obj > -1) {
		Scena[selected_obj].shader = (ShaderType)option;
	}
	glutPostRedisplay();
}

void illumination_menu_function(int option) {
	if (selected_obj > -1) {
		Scena[selected_obj].illumination = (IlluminationType)option;
	}
	glutPostRedisplay();
}

void buildOpenGLMenu()
{
	int materialSubMenu = glutCreateMenu(material_menu_function);

	glutAddMenuEntry(materials[MaterialType::EMERALD].name.c_str(), MaterialType::EMERALD);
	glutAddMenuEntry(materials[MaterialType::BRASS].name.c_str(), MaterialType::BRASS);
	glutAddMenuEntry(materials[MaterialType::SLATE].name.c_str(), MaterialType::SLATE);
	glutAddMenuEntry(materials[MaterialType::YELLOW].name.c_str(), MaterialType::YELLOW);

	int shaderSubMenu = glutCreateMenu(shader_menu_function);
	glutAddMenuEntry(shaders[ShaderType::NO_SHADER].name.c_str(), ShaderType::NO_SHADER);
	glutAddMenuEntry(shaders[ShaderType::TEXTURE].name.c_str(), ShaderType::TEXTURE);
	glutAddMenuEntry(shaders[ShaderType::PHONG_SHADING].name.c_str(), ShaderType::PHONG_SHADING);
	glutAddMenuEntry(shaders[ShaderType::INTERPOLATE_SHADING].name.c_str(), ShaderType::INTERPOLATE_SHADING);

	int illuminationSubMenu = glutCreateMenu(illumination_menu_function);
	glutAddMenuEntry(illuminations[IlluminationType::NO_ILLUMINATION].name.c_str(), IlluminationType::NO_ILLUMINATION);
	glutAddMenuEntry(illuminations[IlluminationType::PHONG].name.c_str(), IlluminationType::PHONG);
	glutAddMenuEntry(illuminations[IlluminationType::BLINN].name.c_str(), IlluminationType::BLINN);

	glutCreateMenu(main_menu_func);	 // richiama main_menu_func() alla selezione di una voce menu
	glutAddMenuEntry("Options", -1);
	glutAddSubMenu("Material", materialSubMenu);
	glutAddSubMenu("Shader", shaderSubMenu);
	glutAddSubMenu("Illumination", illuminationSubMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void drawScene(void)
{
	int i, j, k;
	float traslax, traslay;

	glUniformMatrix4fv(MatrixProj, 1, GL_FALSE, value_ptr(Projection));
	View = lookAt(vec3(SetupTelecamera.position), vec3(SetupTelecamera.target), vec3(SetupTelecamera.upVector));

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	glClearColor(0.0, 0.0, 1.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Disegno Sky box

	glDepthMask(GL_FALSE);

	glUseProgram(programId1);
	glUniform1i(glGetUniformLocation(programId1, "skybox"), 0);
	glUniformMatrix4fv(MatrixProjS, 1, GL_FALSE, value_ptr(Projection));
	glUniformMatrix4fv(MatViewS, 1, GL_FALSE, value_ptr(View));
	// skybox cube
	glBindVertexArray(Scena[0].VAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, Scena[0].indici.size() * sizeof(GLuint), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

	glUseProgram(programId);
	// Utilizzo il program shader per il disegno
	glUniformMatrix4fv(MatrixProjR, 1, GL_FALSE, value_ptr(Projection));
	glUniformMatrix4fv(MatModelR, 1, GL_FALSE, value_ptr(Scena[1].Model));
	glUniformMatrix4fv(MatViewR, 1, GL_FALSE, value_ptr(View));
	glUniform3f(loc_view_posR, SetupTelecamera.position.x, SetupTelecamera.position.y, SetupTelecamera.position.z);
	glBindVertexArray(Scena[1].VAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, (Scena[1].indici.size() - 1) * sizeof(GLuint), GL_UNSIGNED_INT, 0);

	// Cambio program per renderizzare tutto il resto della scena
	glUseProgram(programId);
	// Costruisco la matrice di Vista che applicata ai vertici in coordinate del mondo li trasforma nel sistema di riferimento della camera.

	// Passo al Vertex Shader il puntatore alla matrice View, che sar� associata alla variabile Uniform mat4 Projection
	// all'interno del Vertex shader. Uso l'identificatio MatView
	glPointSize(10.0);
	glUniformMatrix4fv(MatView, 1, GL_FALSE, value_ptr(View));
	// Definizione colore luce, posizione ed intensit
	glUniform3f(light_unif.light_position_pointer, light.position.x + 80 * cos(radians(angolo)), light.position.y, light.position.z + 80 * sin(radians(angolo)));
	glUniform3f(light_unif.light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_unif.light_power_pointer, light.power);

	// Passo la posizione della camera
	glUniform3f(loc_view_pos, SetupTelecamera.position.x, SetupTelecamera.position.y, SetupTelecamera.position.z);
	glUniform1f(loctime, time);

	for (int k = 1; k < Scena.size(); k++)
	{
		// Trasformazione delle coordinate dell'ancora dal sistema di riferimento dell'oggetto in sistema
		// di riferimento del mondo premoltiplicando per la matrice di Modellazione.
		Scena[k].ancora_world = Scena[k].ancora_obj;
		Scena[k].ancora_world = Scena[k].Model * Scena[k].ancora_world;

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[k].Model));
		// Passo al Vertex Shader il puntatore alla matrice Model dell'oggetto k-esimo della Scena, che sar� associata alla variabile Uniform mat4 Projection
		// all'interno del Vertex shader. Uso l'identificatio MatModel
		glUniform1i(locSceltaVs, Scena[k].shader);
		glUniform1i(locIllumination, Scena[k].illumination);

		glUniform3fv(light_unif.material_ambient, 1, glm::value_ptr(materials[Scena[k].material].ambient));
		glUniform3fv(light_unif.material_diffuse, 1, glm::value_ptr(materials[Scena[k].material].diffuse));
		glUniform3fv(light_unif.material_specular, 1, glm::value_ptr(materials[Scena[k].material].specular));
		glUniform1f(light_unif.material_shininess, materials[Scena[k].material].shininess);
		glBindVertexArray(Scena[k].VAO);

		if (Scena[k].sceltaVS != 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, Scena[k].sceltaVS);
			glUniform1i(loc_texture, 1);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, (Scena[k].indici.size() - 1) * sizeof(GLuint), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// Renderizzazione dei modelli obj

	for (int j = 0; j < ScenaObj.size(); j++)
	{
		for (int k = 0; k < ScenaObj[j].size(); k++)
		{
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(ScenaObj[j][k].ModelM));
			glUniform1i(locSceltaVs, ScenaObj[j][k].sceltaVS);
			glUniform1i(locIllumination, ScenaObj[j][k].illumination);
			// Passo allo shader il puntatore ai materiali

			glUniform3fv(light_unif.material_ambient, 1, value_ptr(ScenaObj[j][k].materiale.ambient));
			glUniform3fv(light_unif.material_diffuse, 1, value_ptr(ScenaObj[j][k].materiale.diffuse));
			glUniform3fv(light_unif.material_specular, 1, value_ptr(ScenaObj[j][k].materiale.specular));
			glUniform1f(light_unif.material_shininess, ScenaObj[j][k].materiale.shininess);

			glBindVertexArray(ScenaObj[j][k].VAO);
			glDrawElements(GL_TRIANGLES, (ScenaObj[j][k].indici.size()) * sizeof(GLuint), GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
		}
	}
	// Imposto il renderizzatore del testo
	if (selected_obj > -1) {
		RenderText(programId_text, Projection_text, "Nome: " + Scena[selected_obj].nome, VAO_Text, VBO_Text, 20.0f, 770.0f, 0.5f, vec3(1.0f, 1.0f, 1.0f));
		RenderText(programId_text, Projection_text, "Material: " + materials[Scena[selected_obj].material].name, VAO_Text, VBO_Text, 20.0f, 750.0f, 0.5f, vec3(1.0f, 1.0f, 1.0f));
		RenderText(programId_text, Projection_text, "Shader: " + shaders[Scena[selected_obj].shader].name, VAO_Text, VBO_Text, 20.0f, 730.0f, 0.5f, vec3(1.0f, 1.0f, 1.0f));
		RenderText(programId_text, Projection_text, "Illumination: " + illuminations[Scena[selected_obj].illumination].name, VAO_Text, VBO_Text, 20.0f, 710.0f, 0.5f, vec3(1.0f, 1.0f, 1.0f));
	}
	glutSwapBuffers();
}

void update(int value)
{
	glutTimerFunc(200, update, 0);
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	// Inizializzo finestra per il rendering della scena 3d con tutti i suoi eventi le sue inizializzazioni e le sue impostazioni

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Progetto 2");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);

	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboardPressedEvent);
	glutTimerFunc(200, update, 0);
	glutPassiveMotionFunc(my_passive_mouse);
	glutMotionFunc(mouseActiveMotion);
	glewExperimental = GL_TRUE;

	// Inizializzazioni
	glewInit();
	INIT_SHADER();
	INIT_VAO();
	INIT_CAMERA_PROJECTION();
	INIT_Illuminazione();
	INIT_VAO_Text();
	buildOpenGLMenu();
	inizilizzaCubemap();
	Init_Freetype();
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Projection (in vertex shader).
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice Projection al Vertex Shader
	MatrixProj = glGetUniformLocation(programId, "Projection");
	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model (in vertex shader)
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice Model al Vertex Shader
	MatModel = glGetUniformLocation(programId, "Model");
	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 View (in vertex shader)
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice View al Vertex Shader
	MatView = glGetUniformLocation(programId, "View");

	locSceltaVs = glGetUniformLocation(programId, "sceltaVS");
	locIllumination = glGetUniformLocation(programId, "illumination");
	loctime = glGetUniformLocation(programId, "time");
	loc_view_pos = glGetUniformLocation(programId, "ViewPos");
	loc_texture = glGetUniformLocation(programId, "id_tex");

	light_unif.light_position_pointer = glGetUniformLocation(programId, "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(programId, "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(programId, "light.power");
	light_unif.material_ambient = glGetUniformLocation(programId, "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(programId, "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(programId, "material.specular");
	light_unif.material_shininess = glGetUniformLocation(programId, "material.shininess");

	// location variabili uniformi per lo shader della gestione della cubemap
	MatrixProjS = glGetUniformLocation(programId1, "Projection");
	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model (in vertex shader)
	// QUesto identificativo sar  poi utilizzato per il trasferimento della matrice Model al Vertex Shader

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model (in vertex shader)
	// QUesto identificativo sar  poi utilizzato per il trasferimento della matrice Model al Vertex Shader
	MatViewS = glGetUniformLocation(programId1, "View");
	MatModelR = glGetUniformLocation(programIdr, "Model");
	MatViewR = glGetUniformLocation(programIdr, "View");
	MatrixProjR = glGetUniformLocation(programIdr, "Projection");
	loc_view_posR = glGetUniformLocation(programIdr, "ViewPos");
	loc_cubemapR = glGetUniformLocation(programIdr, "cubemap");
	glutMainLoop();
}