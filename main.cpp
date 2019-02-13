//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

#include "SOIL/SOIL.h"

#include <cmath>
#include <ctime>
#include <cstdlib>

static const GLsizei WIDTH = 1024, HEIGHT = 1024; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse = true;    // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;

static bool normalsVisualize = false;
static bool fog = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(-30.0f, 40.0f, -25.0f));

constexpr float noize = 0.3f;

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
    case GLFW_KEY_4:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    case GLFW_KEY_1:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case GLFW_KEY_2:
        normalsVisualize = true;
        break;
    case GLFW_KEY_3:
        normalsVisualize = false;
        break;
    case GLFW_KEY_5:
        fog = true;
        break;
    case GLFW_KEY_6:
        fog = false;
        break;
    default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        g_captureMouse = !g_captureMouse;


    if (g_captureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_capturedMouseJustNow = true;
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - lastX;
    GLfloat yoffset = lastY - float(ypos);    

    lastX = float(xpos);
    lastY = float(ypos);

    if (g_captureMouse)
        camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

float myrandom(float a, float b) {
    return a + std::rand() * (b - a) / RAND_MAX;
}

float diamond_step (int i, int j, int step_size, std::vector<std::vector<float>> &y) {
    float avg = y[i - step_size][j - step_size] + y[i - step_size][j + step_size] + 
                y[i + step_size][j - step_size] + y[i + step_size][j + step_size];
    y[i][j] = avg / 4;
    y[i][j] += myrandom( -step_size * noize, step_size * noize);
    return y[i][j];
}
// returns max height of new values in height map
float square_step (int i, int j, int step_size, std::vector<std::vector<float>> &y) {
    int size = y.size();
    float maxHeight = 0;
    for (int k = 0; k < 4; ++k) {
        int ii, jj;
        int avg = 0;
        int counter = 0;
        // coordinates of central cell
        if (k < 2) {
            ii = i;
            if (k % 2 == 0) {
                jj = j + step_size;
            } else {
                jj = j - step_size;
            }
        } else {
            jj = j;
            if (k % 2 == 0) {
                ii = i + step_size;
            } else {
                ii = i - step_size;
            }
        }

       if (jj - step_size >= 0) {
            avg += y[ii][jj - step_size];
            counter++;
        }
        if (ii - step_size >= 0) {
            avg += y[ii - step_size][jj];
            counter++;
        }
        if (ii + step_size < size) {
            avg += y[ii + step_size][jj];
            counter++;
        }
        if (jj + step_size < size) {
            avg += y[ii][jj + step_size];
            counter++;
        }
        y[ii][jj] = avg / counter;
        y[ii][jj] += myrandom( -step_size * noize, step_size * noize);
        if (y[ii][jj] > maxHeight) {
            maxHeight = y[ii][jj];
        }
    }
    return maxHeight;
}

void average_normalization (std::vector<std::vector<float>> &heightMap) {
    for (int i = 1; i < heightMap.size() - 1; ++i) {
        for (int j = 1; j < heightMap.size() - 1; ++j) {
            float avg = 0;
            for (int ii = i - 1; ii < i + 2; ++ii) {
                for (int jj = j - 1; jj < j + 2; ++jj) {
                    avg += heightMap[ii][jj];
                }
            }
            heightMap[i][j] = avg / 9;
        }    
    }
}

void print_height_map (std::vector<std::vector<float>> &heightMap) {
    for (int i = 0; i < heightMap.size(); ++i) {
        for (int j = 0; j < heightMap.size(); ++j) {
            std::cout << heightMap[i][j] << " " ;
        }
        std::cout << std::endl;
    }
}

void heght_map_normalization (std::vector<std::vector<float>> &heightMap, float maxHeight) {
    for (int i = 0; i < heightMap.size(); ++i) {
        for (int j = 0; j < heightMap.size(); ++j) {
            heightMap[i][j] /= maxHeight;
            heightMap[i][j] *= heightMap[i][j];
            heightMap[i][j] *= 10;
        }
    }
    average_normalization (heightMap);
}

/**
\brief создать triangle strip плоскость и загрузить её в шейдерную программу
\param rows - число строк
\param cols - число столбцов
\param size - размер плоскости
\param vao - vertex array object, связанный с созданной плоскостью
*/
static int createTriStrip(int n, float size, GLuint &vao)
{
    int rows = std::pow(2, n) + 1;
    int cols = rows;
    int numIndices = 2 * cols*(rows - 1) + rows - 1;

    std::vector<GLfloat> vertices_vec; //вектор атрибута координат вершин
    vertices_vec.reserve(rows * cols * 3);

    std::vector<GLfloat> normals_vec; //вектор атрибута нормалей к вершинам
    normals_vec.reserve(rows * cols * 3);

    std::vector<GLfloat> texcoords_vec; //вектор атрибут текстурных координат вершин
    texcoords_vec.reserve(rows * cols * 2);

    std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f, 0.0f)); //временный вектор нормалей, используемый для расчетов

    std::vector<int3> faces;                 // вектор граней (треугольников), каждая грань - три индекса вершин, её составляющих; 
                                                                     // используется для удобства расчета нормалей
    faces.reserve(numIndices / 3);

    std::vector<GLuint> indices_vec; //вектор индексов вершин для передачи шейдерной программе
    indices_vec.reserve(numIndices);

    std::vector<std::vector<float>> yy;
    yy.resize(rows);
    for(int i = 0; i < rows; ++i) {
        yy[i].resize(cols);
    }
//    std::srand(std::time(nullptr));
    yy[0][0] = myrandom(5, 80);
    yy[0][cols - 1] = myrandom(5, 40);
    yy[rows - 1][0] = myrandom(5, 40);
    yy[rows - 1][cols - 1] = myrandom(5, 40);
    float maxHeight = 0;
//std::cout << yy[0][0] << " " << yy[0][cols - 1] << " " << yy[rows - 1][0] <<" " << yy[rows - 1][cols - 1] << std::endl;

//int step_size = rows / 2;
//    for (int step = rows - 1; step > 1; step /= 2) {
  //      int step_size = (step + 1) / 2;
    for (int step_size = rows / 2; step_size > 0; step_size /= 2) {
        // loop over squares with diamond step
        for (int i = step_size; i < rows; i += 2 * step_size) {
            for (int j = step_size; j < cols; j += 2 * step_size) {
                float h = diamond_step(i, j, step_size, yy);
                if (h > maxHeight) {
                    maxHeight = h;
                }
  //              std::cout << "  i j " << i << " " << j << std::endl;
            }
        }
        // loop over squares with square step
        for (int i = step_size; i < rows; i += 2 * step_size) {
            for (int j = step_size; j < cols; j += 2 * step_size) {
                float h = square_step(i, j, step_size, yy);
                if (h > maxHeight) {
                    maxHeight = h;
                }
            }
        }
    }
    //    print_height_map(yy);
    heght_map_normalization(yy, maxHeight);

    for (int z = 0; z < rows; ++z)
    {
        for (int x = 0; x < cols; ++x)
        {
            //вычисляем координаты каждой из вершин 
            float xx = -size / 2 + x*size / cols;
            float zz = -size / 2 + z*size / rows;

            vertices_vec.push_back(xx);
            vertices_vec.push_back(yy[z][x]);
            vertices_vec.push_back(zz);

            texcoords_vec.push_back(x / float(cols - 1)); // вычисляем первую текстурную координату u, 
                                                          // для плоскости это просто относительное положение вершины
            texcoords_vec.push_back(z / float(rows - 1)); // аналогично вычисляем вторую текстурную координату v
        }
    }

    //primitive restart - специальный индекс, который обозначает конец строки из треугольников в triangle_strip
    //после этого индекса формирование треугольников из массива индексов начнется заново - 
    //будут взяты следующие 3 индекса для первого треугольника и далее каждый последующий индекс будет добавлять 
    //один новый треугольник пока снова не встретится primitive restart index

    int primRestart = cols * rows;

    for (int x = 0; x < cols - 1; ++x)
    {
        for (int z = 0; z < rows - 1; ++z)
        {
            int offset = x*cols + z;

            //каждую итерацию добавляем по два треугольника, которые вместе формируют четырехугольник
            if (z == 0) //если мы в начале строки треугольников, нам нужны первые четыре индекса
            {
                indices_vec.push_back(offset + 0);
                indices_vec.push_back(offset + rows);
                indices_vec.push_back(offset + 1);
                indices_vec.push_back(offset + rows + 1);
            }
            else // иначе нам достаточно двух индексов, чтобы добавить два треугольника
            {
                indices_vec.push_back(offset + 1);
                indices_vec.push_back(offset + rows + 1);

                if (z == rows - 2) indices_vec.push_back(primRestart); // если мы дошли до конца строки, вставляем primRestart, чтобы обозначить переход на следующую строку
            }
        }
    }

    ///////////////////////
    //формируем вектор граней(треугольников) по 3 индекса на каждый
    int currFace = 1;
    for (int i = 0; i < indices_vec.size() - 2; ++i)
    {
        int3 face;

        int index0 = indices_vec.at(i);
        int index1 = indices_vec.at(i + 1);
        int index2 = indices_vec.at(i + 2);

        if (index0 != primRestart && index1 != primRestart && index2 != primRestart)
        {
            if (currFace % 2 != 0) //если это нечетный треугольник, то индексы и так в правильном порядке обхода - против часовой стрелки
            {
                face.x = indices_vec.at(i);
                face.y = indices_vec.at(i + 1);
                face.z = indices_vec.at(i + 2);

                currFace++;
            }
            else //если треугольник четный, то нужно поменять местами 2-й и 3-й индекс;
            {        //при отрисовке opengl делает это за нас, но при расчете нормалей нам нужно это сделать самостоятельно
                face.x = indices_vec.at(i);
                face.y = indices_vec.at(i + 2);
                face.z = indices_vec.at(i + 1);

                currFace++;
            }
            faces.push_back(face);
        }
    }


    ///////////////////////
    //расчет нормалей
    for (int i = 0; i < faces.size(); ++i)
    {
        //получаем из вектора вершин координаты каждой из вершин одного треугольника
        float3 A(vertices_vec.at(3 * faces.at(i).x + 0), vertices_vec.at(3 * faces.at(i).x + 1), vertices_vec.at(3 * faces.at(i).x + 2));
        float3 B(vertices_vec.at(3 * faces.at(i).y + 0), vertices_vec.at(3 * faces.at(i).y + 1), vertices_vec.at(3 * faces.at(i).y + 2));
        float3 C(vertices_vec.at(3 * faces.at(i).z + 0), vertices_vec.at(3 * faces.at(i).z + 1), vertices_vec.at(3 * faces.at(i).z + 2));

        //получаем векторы для ребер треугольника из каждой из 3-х вершин
        float3 edge1A(normalize(B - A));
        float3 edge2A(normalize(C - A));

        float3 edge1B(normalize(A - B));
        float3 edge2B(normalize(C - B));

        float3 edge1C(normalize(A - C));
        float3 edge2C(normalize(B - C));

        //нормаль к треугольнику - векторное произведение любой пары векторов из одной вершины
        float3 face_normal = cross(edge1A, edge2A);

        //простой подход: нормаль к вершине = средняя по треугольникам, к которым принадлежит вершина
        normals_vec_tmp.at(faces.at(i).x) += face_normal;
        normals_vec_tmp.at(faces.at(i).y) += face_normal;
        normals_vec_tmp.at(faces.at(i).z) += face_normal;
    }

    //нормализуем векторы нормалей и записываем их в вектор из GLFloat, который будет передан в шейдерную программу
    for (int i = 0; i < normals_vec_tmp.size(); ++i)
    {
        float3 N = normalize(normals_vec_tmp.at(i));

        normals_vec.push_back(N.x);
        normals_vec.push_back(N.y);
        normals_vec.push_back(N.z);
    }


    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboVertices);
    glGenBuffers(1, &vboIndices);
    glGenBuffers(1, &vboNormals);
    glGenBuffers(1, &vboTexCoords);


    glBindVertexArray(vao); GL_CHECK_ERRORS;
    {

        //передаем в шейдерную программу атрибут координат вершин
        glBindBuffer(GL_ARRAY_BUFFER, vboVertices); GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, vertices_vec.size() * sizeof(GL_FLOAT), &vertices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
        glEnableVertexAttribArray(0); GL_CHECK_ERRORS;

        //передаем в шейдерную программу атрибут нормалей
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals); GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, normals_vec.size() * sizeof(GL_FLOAT), &normals_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
        glEnableVertexAttribArray(1); GL_CHECK_ERRORS;

        //передаем в шейдерную программу атрибут текстурных координат
        glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords); GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, texcoords_vec.size() * sizeof(GL_FLOAT), &texcoords_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
        glEnableVertexAttribArray(2); GL_CHECK_ERRORS;

        //передаем в шейдерную программу индексы
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices); GL_CHECK_ERRORS;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_vec.size() * sizeof(GLuint), &indices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;

        glEnable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
        glPrimitiveRestartIndex(primRestart); GL_CHECK_ERRORS;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


    return numIndices;
}


int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "Controls: "<< std::endl;
    std::cout << "press left mose button to capture/release mouse cursor    "<< std::endl;
    std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
    std::cout << "press ESC to exit" << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if(!glfwInit())
        return -1;

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 


    GLFWwindow*    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); 

	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback                (window, OnKeyboardPressed);    
	glfwSetCursorPosCallback    (window, OnMouseMove); 
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback         (window, OnMouseScroll);
	glfwSetInputMode                    (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	if(initGL() != 0) 
		return -1;
	
    //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]     = "vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;

    
    //Создаем и загружаем геометрию поверхности
    GLuint vaoTriStrip;
    int triStripIndices = createTriStrip(9, 40, vaoTriStrip);


    glViewport(0, 0, WIDTH, HEIGHT);    GL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);    GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//transparence of water 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//

// textures
    GLuint waterTexture, sandTexture, grassTexture, forestTexture, rockTexture, snowTexture;
    unsigned char* image;
    
    glGenTextures(1, &waterTexture);
    glBindTexture(GL_TEXTURE_2D, waterTexture);
    int width, height;
    image = SOIL_load_image("../textures/water0.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    //glGenTextures(1, &sandTexture);
    //glBindTexture(GL_TEXTURE_2D, sandTexture);
    glGenTextures(1, &sandTexture);
    glBindTexture(GL_TEXTURE_2D, sandTexture);
    image = SOIL_load_image("../textures/sand.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

//    glGenTextures(1, &grassTexture);
//    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    image = SOIL_load_image("../textures/grass.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

//    glGenTextures(1, &forestTexture);
//    glBindTexture(GL_TEXTURE_2D, forestTexture);
    glGenTextures(1, &forestTexture);
    glBindTexture(GL_TEXTURE_2D, forestTexture);
    image = SOIL_load_image("../textures/forest.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

//    glGenTextures(1, &rockTexture);
//    glBindTexture(GL_TEXTURE_2D, rockTexture);
    glGenTextures(1, &rockTexture);
    glBindTexture(GL_TEXTURE_2D, rockTexture);
    image = SOIL_load_image("../textures/rock.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

//    glGenTextures(1, &snowTexture);
//    glBindTexture(GL_TEXTURE_2D, snowTexture);
    glGenTextures(1, &snowTexture);
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    image = SOIL_load_image("../textures/snow.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

//
	//цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
		//считаем сколько времени прошло за кадр
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
        doCameraMovement(camera, deltaTime);

		//очищаем экран каждый кадр
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

        program.StartUseShader(); GL_CHECK_ERRORS;

		//обновляем матрицы камеры и проекции каждый кадр
        float4x4 view             = camera.GetViewMatrix();
        float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);

		                                //модельная матрица, определяющая положение объекта в мировом пространстве
		float4x4 model; //начинаем с единичной матрицы
		
        program.StartUseShader();

        //загружаем uniform-переменные в шейдерную программу (одинаковые для всех параллельно запускаемых копий шейдера)
        program.SetUniform("view",             view); GL_CHECK_ERRORS;
        program.SetUniform("projection", projection); GL_CHECK_ERRORS;
        program.SetUniform("model",           model);
        program.SetUniform("mode", normalsVisualize);
        program.SetUniform("fog",               fog);

        //рисуем плоскость
        glBindVertexArray(vaoTriStrip);
//textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTexture);
        program.SetUniform("WaterTexture", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        program.SetUniform("GrassTexture", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, forestTexture);
        program.SetUniform("ForestTexture", 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, rockTexture);
        program.SetUniform("RockTexture", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, sandTexture);
        program.SetUniform("SandTexture", 4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, snowTexture);
        program.SetUniform("SnowTexture", 5);
//
        glDrawElements(GL_TRIANGLE_STRIP, triStripIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        glBindVertexArray(0); GL_CHECK_ERRORS;

        program.StopUseShader();

		glfwSwapBuffers(window); 
	}

	//очищаем vao перед закрытием программы
	glDeleteVertexArrays(1, &vaoTriStrip);

	glfwTerminate();
	return 0;
}
