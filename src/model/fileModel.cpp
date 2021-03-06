#include "model.hpp"
#include "scene.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include <fstream>

int FileModel::instance = 0;

FileModel::FileModel(std::string _path, SMOOTH_NORMAL _smoothNormals){

	name = _path.substr(_path.find_last_of("/")+1)+"_"+std::to_string(instance);
	instance++;
	
	Assimp::Importer importer;

	std::cout << "loading model from file : " 
	<<_path<< " ..." << std::endl;
	
	const aiScene *scene = _smoothNormals?
		importer.ReadFile(_path, 
			  aiProcess_Triangulate 
			| aiProcess_FlipUVs
			| aiProcess_GenSmoothNormals):
		importer.ReadFile(_path, 
			  aiProcess_Triangulate 
			| aiProcess_FlipUVs
			| aiProcess_GenNormals);


	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

	//std::string directory = _path.substr(0, _path.find_last_of('/'));

	
	subModels.resize(scene->mNumMeshes);
	//process each mesh of the model

	std::cout << "processing "<< subModels.size()<< " submeshes ..." << std::endl;

	// configure each mesh
    for(size_t i = 0; i < scene->mNumMeshes; i++)
    {
		aiMesh *mesh = scene->mMeshes[i];
		subModels[i].translate = glm::mat4{1.0};
		subModels[i].scale     = glm::mat4{1.0};
		subModels[i].rotation  = glm::mat4{1.0};
        processMesh(mesh, scene,i);
	}

	//center objects vertices
	double meanX = 0, meanY = 0, meanZ = 0;
	size_t count = 0;
	for(auto& subModel :subModels){
		for(size_t i = 0; i< subModel.vertices.size();i+=3){
			count++;
			meanX+=subModel.vertices[i];
			meanY+=subModel.vertices[i+1];
			meanZ+=subModel.vertices[i+2];
		}
	}
	
	meanX /= count;
	meanY /= count;
	meanZ /= count;
	for(auto& subModel :subModels){
		for(size_t i = 0; i<subModel.vertices.size();i+=3){
			count++;
			subModel.vertices[i] -= (GLfloat)meanX;
			subModel.vertices[i+1] -= (GLfloat)meanY;
			subModel.vertices[i+2] -= (GLfloat)meanZ;
		}
	}

	//subtract min

}

//void FileModel::processMaterial(){
//
//}

void FileModel::processMesh(aiMesh *_mesh, const aiScene *_scene, size_t _meshIdx){
	auto& vert = subModels[_meshIdx].vertices;
	auto& ind = subModels[_meshIdx].indices;
	auto& norm = subModels[_meshIdx].normals;
	//auto& tex = subModels[_meshIdx].textureCoord;

	// add vertices
	for(size_t i = 0; i < _mesh->mNumVertices; i++){
		vert.insert(vert.end(),
			{_mesh->mVertices[i].x,_mesh->mVertices[i].y,_mesh->mVertices[i].z});
    }
	// add normals
	for(size_t i = 0; i < _mesh->mNumVertices; i++){
		norm.insert(norm.end(),
			{_mesh->mNormals[i].x,_mesh->mNormals[i].y,_mesh->mNormals[i].z});
    }
	// add indices
	for(size_t i = 0; i < _mesh->mNumFaces; i++){
    	aiFace face = _mesh->mFaces[i];
    	for(size_t j = 0; j < face.mNumIndices; j++)
        ind.push_back(face.mIndices[j]);
	}

}

//! Load vertex buffers and shader of cube
void FileModel::load(){
	for(auto& subModel : subModels){

		// gen geometry buffers
    	glGenBuffers(1, &subModel.vbo);
    	glGenBuffers(1, &subModel.nbo);
    	glGenBuffers(1, &subModel.tbo);
		glGenBuffers(1, &subModel.ebo);
    	glGenVertexArrays(1, &subModel.vao);

		// Bind the vao
    	glBindVertexArray(subModel.vao);

		//copy indices to vbo
    	glBindBuffer(GL_ARRAY_BUFFER, subModel.vbo);
    	glBufferData(GL_ARRAY_BUFFER, subModel.vertices.size() * sizeof(GLfloat), subModel.vertices.data(), GL_STATIC_DRAW);
		// define array for vertices
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    	glEnableVertexAttribArray(0);


    	// copy normals to nbo
    	glBindBuffer(GL_ARRAY_BUFFER, subModel.nbo);
    	glBufferData(GL_ARRAY_BUFFER, subModel.normals.size() * sizeof(GLfloat), subModel.normals.data(), GL_STATIC_DRAW);
    	// define array for normals
    	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    	glEnableVertexAttribArray(1);

		//// Copy texture coordinates array in element buffer
    	//glBindBuffer(GL_ARRAY_BUFFER, subModel.tbo);
    	//glBufferData(GL_ARRAY_BUFFER, subModel.textureCoord.size() * sizeof(GLfloat), subModel.textureCoord.data(), GL_STATIC_DRAW);
		//// define array for texture coordinates
    	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    	//glEnableVertexAttribArray(2);
		
		
		//copy indices to ebo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,subModel.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, subModel.indices.size() * sizeof(GLfloat), subModel.indices.data(), GL_STATIC_DRAW);

    	// Unbind the VAO
    	glBindVertexArray(0);

		// load right shader

		if ((subModel.diffuseMapPath != "") ){
			subModel.diffuseMap = loadTexture(subModel.diffuseMapPath.c_str());
		}
		if(subModel.specularMapPath != ""){
			subModel.specularMap = loadTexture(subModel.specularMapPath.c_str());
		}
		if (subModel.heightMapPath != ""){
			subModel.heightMap = loadTexture(subModel.heightMapPath.c_str());
		}
		if (subModel.normalMapPath != ""){
			subModel.normalMap = loadTexture(subModel.normalMapPath.c_str());
		}
		if (subModel.AOMapPath != ""){
			subModel.AOMap = loadTexture(subModel.AOMapPath.c_str());
		}
		if(subModel.metallicMapPath != ""){
			subModel.metallicMap = loadTexture(subModel.metallicMapPath.c_str());
		}
		loadShaders(subModel);
	}
}


void FileModel::render(Scene* _scene)  {
	Camera& cam = _scene->getCam();
	SSAO* ssao =  _scene->getSSAO();
	std::vector<Light*>& lights =  _scene->getLights();
	for(auto& subModel : subModels){
		
		auto& sh = subModel.shader;
    	sh.use();
	
    	glm::mat4 model = subModel.translate*subModel.rotation*subModel.scale;

		sh.setMat4("model", model);
    	sh.setMat4("view",cam.getView());
    	sh.setMat4("projection", cam.getProj());
		sh.setVec2("screenSize", glm::vec2(cam.getResWidth(),cam.getResHeight()));
    	sh.setVec3("viewPos", cam.getPos());
		sh.setFloat("exposure",_scene->getExposure());
		sh.setVec2("texScaling", subModel.texScaling);

		sh.setBool("material.hasTexture",false);
		sh.setBool("material.hasNormalMap",false);
		sh.setBool("material.hasMetallicTex",false);

		
		if(subModel.shaderType == PHONG){
			sh.setFloat("material.specularStrength", 1.0f);
			sh.setFloat("material.shininess", subModel.shininess);
			sh.setVec3("material.diffuse", subModel.diffuseColor);
			sh.setVec3("material.specular", subModel.specularColor);
		} else if (subModel.shaderType == PBR){
			sh.setVec3("material.albedo", subModel.diffuseColor);
			sh.setFloat("material.roughness", subModel.roughness);
			sh.setFloat("material.metallic",subModel.metallic);
		}	

		// bind SSAO texture if enabled
		if (_scene->SSAOstatus()){
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssao->getSSAOBlurTexture());
			sh.setInt("SSAOTexture", 0);
			sh.setBool("SSAOenabled",true);
		} else {
			sh.setBool("SSAOenabled", false);
		}

		// point lights properties
		int j = 0;
    	for(uint32_t i = 0; i<std::min(lights.size(),(size_t)MAXLIGHTS); i++){
			if(lights[i]->hasShadowMap()){
				glActiveTexture(GL_TEXTURE1+j);
				DistantLight* li = dynamic_cast<DistantLight*>(lights[i]);
				glBindTexture(GL_TEXTURE_2D, li->getDepthTexture());

				sh.setMat4("lightSpaceMatrix["+std::to_string(j)+"]", li->getLightSpacematrix());
				sh.setInt("shadowMap["+std::to_string(j)+"]", 1+j);
				sh.setInt("lights["+   std::to_string(i) + "].shadowMapId", j);
				j++;
			} else {
				sh.setInt("lights["+   std::to_string(i) + "].shadowMapId", -1);
			}

			if(lights[i]->isDistant()){
				sh.setBool("lights["+   std::to_string(i) + "].distant",1);
			} else {
				sh.setBool("lights["+   std::to_string(i) + "].distant",0);
			}
			
			sh.setBool("lights["+   std::to_string(i) + "].enabled",1);

    	    sh.setVec3("lights["+   std::to_string(i) + "].position",  lights[i]->getPos());
			if(subModel.shaderType == PHONG){
    	    sh.setVec3("lights[" +  std::to_string(i) + "].ambient",   lights[i]->getAmbiant());
		    sh.setVec3("lights[" +  std::to_string(i) + "].diffuse",   lights[i]->getDiffuse());
		    sh.setVec3("lights[" +  std::to_string(i) + "].specular",  lights[i]->getSpecular());

		    sh.setFloat("lights[" + std::to_string(i) + "].constant",  lights[i]->getConstant());
		    sh.setFloat("lights[" + std::to_string(i) + "].linear",    lights[i]->getLinear());
		    sh.setFloat("lights[" + std::to_string(i) + "].quadratic", lights[i]->getQuadratic());
			} 
			else if (subModel.shaderType == PBR){
				sh.setVec3("lights[" +  std::to_string(i) + "].color",   lights[i]->getDiffuse());
			}
		}
		if (lights.size()<MAXLIGHTS){
			for (size_t i = lights.size(); i<MAXLIGHTS; i++){
				sh.setBool("lights["+   std::to_string(i) + "].enabled",0);
			}
		}

		if(subModel.tqual != DISABLED){
			//level of detail based on distance , adapts to any triangle size
			if(subModel.tqual == LOW){
				sh.setInt("tes_lod0", 16); //under 2 unit distance
				sh.setInt("tes_lod1", 4); //2 to 5 distance
				sh.setInt("tes_lod2", 2);  // farther than 5
			} else if (subModel.tqual == MEDIUM){
				sh.setInt("tes_lod0", 32); //under 2 unit distance
				sh.setInt("tes_lod1", 8); //2 to 5 distance
				sh.setInt("tes_lod2", 2);  // farther than 5
			} else if(subModel.tqual == HIGH){
				sh.setInt("tes_lod0", 64); //under 2 unit distance
				sh.setInt("tes_lod1", 16); //2 to 5 distance
				sh.setInt("tes_lod2", 4);  // farther than 5
			}
		}
		
    	glBindVertexArray(subModel.vao);
		
		if(subModel.tqual != DISABLED){
			glDrawElements( GL_PATCHES, subModel.indices.size(), GL_UNSIGNED_INT, nullptr);
		} else {
			glDrawElements( GL_TRIANGLES, subModel.indices.size(), GL_UNSIGNED_INT, nullptr);
		}
    	
		glBindVertexArray(0);
	}
}

void FileModel::renderForDepth(Shader& _shader){
	for(auto& subModel : subModels){
	_shader.use();
	glm::mat4 model = subModel.translate*subModel.rotation*subModel.scale;

	_shader.setMat4("model", model);

	glBindVertexArray(subModel.vao);
	
	glDrawElements(GL_TRIANGLES , subModel.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
}

void FileModel::loadShaders(modelDescription& model){
	// load right shader
	std::string frag;
	if(model.shaderType == PHONG){
		frag = "shaders/phong.frag";
	} else if (model.shaderType == PBR){
		frag = "shaders/pbr.frag";
	}
	if(model.tqual != DISABLED){
		model.shader = {"shaders/tessellation/tess.vert",frag,
				 	"shaders/tessellation/tessPN.tesc",
				 	"shaders/tessellation/tessPN.tese"};
		
	} else {
		model.shader = {"shaders/default.vert", frag};
	}
}

void FileModel::loadShaders(){
	for(auto& subModel: subModels)
		loadShaders(subModel);
}


FileModel& FileModel::setScale(glm::vec3  _scale){
	for(auto& subModel : subModels){
    subModel.scale = glm::mat4(1.0);
    subModel.scale = glm::scale(subModel.scale,_scale);
	}
    return *this;
}

FileModel& FileModel::setRotation(float _angle,glm::vec3 _axis){
	for(auto& subModel : subModels){
    subModel.rotation = glm::mat4(1.0);
    subModel.rotation = glm::rotate(subModel.rotation, glm::radians(_angle),_axis);
	}
    return *this;
}

FileModel& FileModel::setPosition(glm::vec3 _pos){
	for(auto& subModel : subModels){
    subModel.translate = glm::mat4{1.0};
    subModel.translate = glm::translate(subModel.translate, _pos);
	}
    return *this;
}



FileModel& FileModel::setDiffuse(glm::vec3 _color){
	for(auto& subModel : subModels){
    subModel.diffuseColor =_color;
	}
	return *this;
}

FileModel& FileModel::setSpecular(glm::vec3 _color){
	for(auto& subModel : subModels){
    subModel.specularColor = _color;
	}
	return *this;
}

FileModel& FileModel::setAlbedo(glm::vec3 _color){
	for(auto& subModel : subModels){
	subModel.diffuseColor = _color;
	}
	return *this;
}

FileModel& FileModel::setRoughness(float _roughness){
	for(auto& subModel : subModels){
		subModel.roughness = _roughness;
	}
	return *this;
}

FileModel& FileModel::setMetallic(float _metallic){
	for(auto& subModel : subModels){
		subModel.metallic = _metallic;
	}
	return *this;
}

FileModel& FileModel::enableTesselation(){
	for(auto& subModel : subModels){
    	subModel.tqual = MEDIUM;
	}
    return *this;
}

FileModel& FileModel::disableTesselation(){
	for(auto& subModel : subModels){
    	subModel.tqual = DISABLED;
	}
    return *this;
}

FileModel& FileModel::enableTesselation(TESS_QUALITY _quality){
	for(auto& subModel : subModels){
		subModel.tqual = _quality;
	}
    return *this;
}

FileModel& FileModel::setShaderType(SHADER_TYPE _type){
	for(auto& subModel : subModels){
		subModel.shaderType = _type;
	}
	return *this;
}

FileModel& FileModel::setShininess(float _shininess){
	for(auto& subModel : subModels){
		subModel.shininess = _shininess;
	}
	return *this;
}
