#version 330 core
in vec4 ourColor;
out vec4 FragColor;

in vec3 N, L, R, V;
uniform vec2 resolution;
uniform int sceltaVS;
uniform int illumination;

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

 //definizione di una variabile uniform che ha la struttura PointLight
uniform PointLight light;
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

uniform sampler2D id_tex;
in vec2 frag_coord_st;

void main() {
    if(sceltaVS == 0 || sceltaVS == 3) { //No shading, oppure shading interpolativo senza texture
        FragColor = ourColor;
    }
    if(sceltaVS == 1) {  //Caso no-shading, shading interpolativo
        //Per sovrappore le due texture
        //FragColor = mix(texture(id_tex1,frag_coord_st),texture(id_tex,frag_coord_st), 0.3);
        FragColor = texture(id_tex, frag_coord_st);
    }
    if(sceltaVS == 2) //Shading di Phong: il modello di illuminazione viene implementato nel fragment shader
    {
        vec3 ambient = light.power * material.ambient;

        //diffuse
        float coseno_angolo_theta = max(dot(L, N), 0);
        vec3 diffuse = light.power * light.color * coseno_angolo_theta * material.diffuse;

        //speculare
        vec3 specular;
        
        // Illuminazione Phong
        if (illumination == 1) {
            vec3 reflection_dir = reflect(-L, N);
            float specularPhong = pow(max(dot(reflection_dir, V), 0.0), material.shininess);
            specular = light.power * light.color * specularPhong * material.specular;
        }
        // Illuminazione Blinn-Phong
        else if (illumination == 2) {
            vec3 angolo_alfa_dir = normalize(L + V);
            float coseno_angolo_alfa = max(dot(N, angolo_alfa_dir), 0.0);
            float specularBlinnPhong = pow(coseno_angolo_alfa, material.shininess);
            specular = light.power * light.color * specularBlinnPhong * material.specular;
        }

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
}