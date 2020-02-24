#include "Window.h"

//general variables
int DrawAreaHeight = 1080;
float ScreenRatio = (float)16/9;
float MouseSensitivity = 0.5f;

//playerspeed is set in moveplayer() dynamically
float PlayerSpeed = 0.0f;
int DrawAreaWidth = 0;
bool ReadFirstMousePos = false;
MouseData lastMousePos = MouseData(0, 0);
CameraData camDir = CameraData(0, 0);
PlayerData pd = PlayerData(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0));
bool KeysDown[] = { false, false, false, false, false, false, false, false, false }; //wasdqe and mouse left / right and space for jumping

//only for fps measuring
double LastTimeM = 0.0;
double LastTime = 0.0;
int NumberFrames = 0;

//the actual frame length
double DeltaTime = 0.0;

//render variables
GLuint bufferID;
GLuint shaderProgramID;

//global shader variables
GLuint playerPos;
GLuint viewFwd;
GLuint viewRight;
GLuint viewUp;
GLuint inverseScreenRatio;

//textures
const int TexNumber = 2;
GLuint Textures[TexNumber];

int Main()
{
	
	//set drawarea
	DrawAreaWidth = ScreenRatio * DrawAreaHeight;
	//pd.Pos = Vector3(13245, 0, 13578);

	GLFWwindow* window;

	//Initialize the library
	if (!glfwInit())
		return -1;

	//set hints
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(DrawAreaWidth, DrawAreaHeight, "Voxel Engine", glfwGetPrimaryMonitor(), NULL); //for window replace primary monitor with NULL
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	
	//Make the window's context current
	glfwMakeContextCurrent(window);

	//init glew
	GLenum glewError = glewInit();
	if (glewError != 0)
		return -1;

	if (!GLEW_VERSION_4_5)
	{
		printf("%s", "Es muss mindestens OpenGL 4.5 installiert sein");
		return -1;
	}
	//turn on off vScreenSizeFctync
	glfwSwapInterval(0);

	//initialize rendering and window callback and set shader and textures
	InitGL(window);

	//hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // to lock GLFW_CURSOR_DISABLED, hide with GLFW_CURSOR_HIDDEN
	
	//set lasttime for deltatime at start
	LastTime = glfwGetTime();

	//Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		//compute input
		SetPlayerViewDir(camDir, pd);
		MovePlayer(pd);

		//set shader variables
		glUniform3f(playerPos, pd.Pos.x, pd.Pos.y, pd.Pos.z);
		glUniform3f(viewFwd, pd.viewDir.x, pd.viewDir.y, pd.viewDir.z);
		glUniform3f(viewRight, pd.viewRight.x, pd.viewRight.y, pd.viewRight.z);
		glUniform3f(viewUp, pd.viewUp.x, pd.viewUp.y, pd.viewUp.z);
		glUniform1f(inverseScreenRatio, 1.0 / ScreenRatio);

		//clear window
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/*
		//activate textures
		for (int i = 0; i < TexNumber; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, Textures[i]);
		}
		*/
		//draw quad for cuda texture and draw, enable scaling of projection matrix
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, 0);

		glTexCoord2f(0, 1);
		glVertex3f(0, DrawAreaHeight, 0);

		glTexCoord2f(1, 1);
		glVertex3f(DrawAreaWidth, DrawAreaHeight, 0);

		glTexCoord2f(1, 0);
		glVertex3f(DrawAreaWidth, 0, 0);
		glEnd();

		//Getfps
		GetFPS(window);

		//Swap front and back buffers
		glfwSwapBuffers(window);

		//Poll for and process events
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}
void InitGL(GLFWwindow* window)
{
	//set window callbacks
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetKeyCallback(window, KeyCallback);

	//init gl
	glViewport(0, 0, DrawAreaWidth, DrawAreaHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, DrawAreaWidth, 0, DrawAreaHeight, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	//init shader
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	//the fragment shader
	char *ShaderFileName = "GLSLfragmentShader.txt";
	string FragmentShader = readShader(ShaderFileName);
	const char *FragmentShaderSource = FragmentShader.c_str();

	//compile programm
	glShaderSource(fragmentShaderID, 1, &FragmentShaderSource, NULL);
	glCompileShader(fragmentShaderID);

	//Check for errors and create log
	GLint fCompileResult;
	GLint fErrorLogLength;
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &fCompileResult);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &fErrorLogLength);

	//create the compile error log
	GLchar *fErrorLog = new GLchar[fErrorLogLength];
	glGetShaderInfoLog(fragmentShaderID, fErrorLogLength, &fErrorLogLength, fErrorLog);

	printf("%s", "Shader Compile Log:\n");
	printf("%s", fErrorLog);

	//create gpu programm
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, fragmentShaderID);
	glLinkProgram(shaderProgramID);
	glDetachShader(shaderProgramID, fragmentShaderID);
	glDeleteShader(fragmentShaderID);
	glUseProgram(shaderProgramID);

	//get variables 
	playerPos = glGetUniformLocation(shaderProgramID, "playerPos");
	viewFwd = glGetUniformLocation(shaderProgramID, "playerFwd");
	viewRight = glGetUniformLocation(shaderProgramID, "playerRight");
	viewUp = glGetUniformLocation(shaderProgramID, "playerUp");
	inverseScreenRatio = glGetUniformLocation(shaderProgramID, "inverseScreenRatio");

	//set textures
	SetTexture(terrainHFile, "TerrainHeigth", GL_REPEAT, GL_LINEAR, 0, GL_RED, GL_R8);
	SetTexture(terrainNFile, "TerrainNormal", GL_REPEAT, GL_LINEAR, 1, GL_RGBA, GL_RGBA8);

	//init some structs
	camDir.yAng = 90;
	pd.Pos = Vector3(-20, 0, 0);

}

void MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
	//void needs to run once to become active
	if (ReadFirstMousePos == true)
	{
		//get the diff to last mousepos
		float xDiff = xpos - lastMousePos.xPos;
		float yDiff = ypos - lastMousePos.yPos;

		//set the cameradata values according to the frame speed
		camDir.xAng -= xDiff * MouseSensitivity;
		camDir.yAng += yDiff * MouseSensitivity;

		//make coords overlap and set borders
		if (camDir.xAng >= 360) camDir.xAng -= 360;
		if (camDir.xAng < 0) camDir.xAng += 360;

		if (camDir.yAng > 179) camDir.yAng = 179;
		if (camDir.yAng < 1) camDir.yAng = 1;
	}
	else ReadFirstMousePos = true;

	//set lastmousepos
	lastMousePos = MouseData(xpos, ypos);
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//close
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//get the keys Down (wasd)
	if (key == GLFW_KEY_W && (action == GLFW_PRESS))
	{
		//y forward
		KeysDown[0] = true;
	}
	if (key == GLFW_KEY_S && (action == GLFW_PRESS))
	{
		//y back
		KeysDown[1] = true;
	}
	if (key == GLFW_KEY_A && (action == GLFW_PRESS))
	{
		//x left
		KeysDown[2] = true;
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS))
	{
		//x right
		KeysDown[3] = true;
	}
	if (key == GLFW_KEY_Q && (action == GLFW_PRESS))
	{
		//up
		KeysDown[4] = true;
	}
	if (key == GLFW_KEY_E && (action == GLFW_PRESS))
	{
		//down
		KeysDown[5] = true;
	}
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS))
	{
		//down
		KeysDown[6] = true;
	}
	
	//get the keys up (wasd)
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		//y forward
		KeysDown[0] = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		//y back
		KeysDown[1] = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		//x left
		KeysDown[2] = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		//x right
		KeysDown[3] = false;
	}
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
	{
		//up
		KeysDown[4] = false;
	}
	if (key == GLFW_KEY_E && action == GLFW_RELEASE)
	{
		//down
		KeysDown[5] = false;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
	{
		//down
		KeysDown[6] = false;
	}
}
void GetFPS(GLFWwindow *window)
{
	// Measure speed
	double CurrentTime = glfwGetTime();
	DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;
	NumberFrames++;

	//check for display every second
	if (CurrentTime - LastTimeM >= 1.0){ // If last frame measure was more than 1 sec ago
		//printf("%d", NumberFrames);
		NumberFrames = 0;
		LastTimeM += 1.0;
	}
}
string readShader(char *filePath) 
{
	string content;
	ifstream fileStream(filePath, ios::in);

	if (!fileStream.is_open()) {
		cerr << "Could not read file " << filePath << ". File does not exist." << endl;
		return "";
	}

	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}
int ReadAllBytes(char const* FileName, vector<char>& File) //0 = no error, 1 = error (file does not exist)
{
	ifstream ifs(FileName, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();
	if ((int)ifs.tellg() == -1) return -1;

	File.resize(pos);

	ifs.seekg(0, ios::beg);
	ifs.read(&File[0], pos);

	return 0;
}
int WriteAllBytes(char const* FileName, vector<char>& File) // creates new file or ovverides
{
	ofstream ofs(FileName, ios::binary);
	ofstream::pos_type pos = File.size();

	ofs.seekp(0, ios::beg);
	ofs.write(&File[0], pos);

	return 0;
}
int LoadTexFromFile(char const* FileName, Texture& Tex)
{
	vector<char> Data;
	int Error = ReadAllBytes(FileName, Data);
	if (Error == -1) return - 1;
	short Width = (Data[1] << 8 | Data[0]);
	short Heigth = (Data[3] << 8 | Data[2]);
	Data.erase(Data.begin(), Data.begin() + 4);
	Tex.Width = Width;
	Tex.Heigth = Heigth;
	Tex.Data = new unsigned char[Data.size()];
	memcpy(Tex.Data, &Data[0], Data.size() * sizeof(char)); // the byte size
}
void SetTexture(const char* FileName, const char *TexName, GLint WarpMode, GLint InterplMode, int TextureChannel, GLenum ColorFormat, GLint BitFormat)
{
	Texture dTex = Texture(0, 0);
	int Error = LoadTexFromFile(FileName, dTex);

	glGenTextures(1, &Textures[TextureChannel]);
	glBindTexture(GL_TEXTURE_2D, Textures[TextureChannel]);

	//set texture params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, InterplMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, InterplMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WarpMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WarpMode);
	
	glTexImage2D(GL_TEXTURE_2D, 0, BitFormat, dTex.Width, dTex.Heigth, 0, ColorFormat, GL_UNSIGNED_BYTE, dTex.Data);
	GLuint TexLoc = glGetUniformLocation(shaderProgramID, TexName);
	glUniform1i(TexLoc, TextureChannel);
}
void SetPlayerViewDir(CameraData camData, PlayerData& playerData)
{
	playerData.viewDir = Normalize(Vector3(sin(DegToRad(camData.yAng)) * cos(DegToRad(camData.xAng)), sin(DegToRad(camData.yAng)) * sin(DegToRad(camData.xAng)), cos(DegToRad(camData.yAng))));
	playerData.viewRight = Normalize(Vector3(sin(DegToRad(camData.yAng)) * cos(DegToRad(camData.xAng - 90)), sin(DegToRad(camData.yAng)) * sin(DegToRad(camData.xAng - 90)), 0));
	playerData.viewUp = Normalize(Cross(playerData.viewDir, playerData.viewRight));

}
void MovePlayer(PlayerData& playerData)
{
	if (KeysDown[0] == true)
	{
		//forward
		pd.Pos = pd.Pos + playerData.viewDir * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[1] == true)
	{
		//back
		pd.Pos = pd.Pos - playerData.viewDir * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[2] == true)
	{
		//left
		pd.Pos = pd.Pos - playerData.viewRight * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[3] == true)
	{
		//right
		pd.Pos = pd.Pos + playerData.viewRight * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[4] == true)
	{
		//left
		pd.Pos = pd.Pos - playerData.viewUp * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[5] == true)
	{
		//right
		pd.Pos = pd.Pos + playerData.viewUp * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[6] == true)
	{
		//acceleration
		PlayerSpeed = 25.0;
	}
	else
	{
		PlayerSpeed = 5.0;
	}
}