#include "Widgets/GameWidget.h"
#include "Application/WApplication.h"

#include <Wt/WText>
#include <Wt/WTimer>
#include <Wt/WImage>
#include <Wt/WString>
#include <Wt/WMemoryResource>

namespace DB
{
	const std::vector<float> GameWidget::_triangleVertices = {
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};
	const std::vector<float> GameWidget::_squareVertices = {
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	GameWidget::GameWidget(Wt::WContainerWidget *parent)
		: Wt::WGLWidget(parent)
	{
		setInline(false);
		setRenderOptions(Wt::WGLWidget::ClientSideRendering | Wt::WGLWidget::AntiAliasing);

		WApplication *app = WApplication::instance();
		app->keyStateUpdated().connect(this, &GameWidget::handleKeyStateChanged);

		_jsMatrix = JavaScriptMatrix4x4();
		addJavaScriptMatrix4(_jsMatrix);
	}

	void GameWidget::initializeGL()
	{
		enableClientErrorChecks();

		Wt::WMatrix4x4 worldTransform;
		worldTransform.lookAt(
			0, 0, 10, //camera position
			0, 0, 0, //looking at
			0, 1, 0); //'up' vector

		if(!_initialized)
		{
			initJavaScriptMatrix4(_jsMatrix);
			setJavaScriptMatrix4(_jsMatrix, worldTransform);

			setClientSideLookAtHandler(_jsMatrix, // the name of the JS matrix
				0, 0, 0,                       // the center point
				0, 1, 0,                          // the up direction
				0.005, 0.005);                    // 'speed' factors

			_initialized = true;
		}

		//First, load a simple shader
		Shader fragmentShader = createShader(FRAGMENT_SHADER);
		shaderSource(fragmentShader, Wt::WString::tr("FragmentShaderSrc").toUTF8());
		compileShader(fragmentShader);
		Shader vertexShader = createShader(VERTEX_SHADER);
		shaderSource(vertexShader, Wt::WString::tr("VertexShaderSrc").toUTF8());
		compileShader(vertexShader);
		_shaderProgram = createProgram();
		attachShader(_shaderProgram, vertexShader);
		attachShader(_shaderProgram, fragmentShader);
		linkProgram(_shaderProgram);
		useProgram(_shaderProgram);

		//Extract the references to the attributes from the shader.
		_vertexPositionAttribute = getAttribLocation(_shaderProgram, "aVertexPosition");
		enableVertexAttribArray(_vertexPositionAttribute);

		//Extract the references the uniforms from the shader
		_pMatrixUniform = getUniformLocation(_shaderProgram, "uPMatrix");
		_mvMatrixUniform = getUniformLocation(_shaderProgram, "uMVMatrix");
		_cMatrixUniform = getUniformLocation(_shaderProgram, "uCMatrix");

		//Create a Vertex Buffer Object (VBO) and load all polygon's data
		//SQUARES
		_squareVertexPositionBuffer = createBuffer();
		bindBuffer(ARRAY_BUFFER, _squareVertexPositionBuffer);

		Wt::WMemoryResource *mem1 = new Wt::WMemoryResource("application/octet", this);
		mem1->setData(reinterpret_cast<const unsigned char*>(&(_squareVertices[0])), static_cast<int>(_squareVertices.size()) * sizeof(float));

		ArrayBuffer clientBufferResource1 = createAndLoadArrayBuffer(mem1->generateUrl());
		bufferData(ARRAY_BUFFER, clientBufferResource1, STATIC_DRAW);

		//TRIANGLES
		_triangleVertexPositionBuffer = createBuffer();
		bindBuffer(ARRAY_BUFFER, _triangleVertexPositionBuffer);

		Wt::WMemoryResource *mem2 = new Wt::WMemoryResource("application/octet", this);
		mem2->setData(reinterpret_cast<const unsigned char*>(&(_triangleVertices[0])), static_cast<int>(_triangleVertices.size()) * sizeof(float));

		ArrayBuffer clientBufferResource2 = createAndLoadArrayBuffer(mem2->generateUrl());
		bufferData(ARRAY_BUFFER, clientBufferResource2, STATIC_DRAW);

		//Set the clear color to a black background
		clearColor(0, 0, 0, 1);

		//Reset Z-buffer, enable Z-buffering
		clearDepth(1);
		enable(DEPTH_TEST);
		depthFunc(LEQUAL);
	}

	void GameWidget::resizeGL(int width, int height)
	{
		//Set the viewport size.
		viewport(0, 0, width, height);

		//Set projection matrix to some fixed values
		Wt::WMatrix4x4 proj;
		proj.perspective(45, ((double)width) / height, 0.1, 100);
		uniformMatrix4(_pMatrixUniform, proj);
	}

	void GameWidget::paintGL()
	{
		//Clear color an depth buffers
		clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

		uniformMatrix4(_cMatrixUniform, _jsMatrix);

		Wt::WMatrix4x4 modelMatrix;
		modelMatrix.setToIdentity();

		//TRIANGLE
		modelMatrix.translate(-1.5, 0.0, -7.0);
		uniformMatrix4(_mvMatrixUniform, modelMatrix);
		
		bindBuffer(ARRAY_BUFFER, _triangleVertexPositionBuffer);
		vertexAttribPointer(_vertexPositionAttribute,
			3,		//size: Every vertex has an X, Y anc Z component
			FLOAT,	//type: They are floats
			false,	//normalized: Please, do NOT normalize the vertices
			3 * 4,	//stride: 3points * 4bytes
			0);		//offset: The byte position of the first vertex in the buffer is 0.
		drawArrays(TRIANGLES, 0, static_cast<int>(_triangleVertices.size()) / 3);

		//SQUARE
		modelMatrix.translate(3.0, 0.0, 0.0);
		uniformMatrix4(_mvMatrixUniform, modelMatrix);

		bindBuffer(ARRAY_BUFFER, _squareVertexPositionBuffer);
		vertexAttribPointer(_vertexPositionAttribute,
			3,		//size: Every vertex has an X, Y anc Z component
			FLOAT,	//type: They are floats
			false,	//normalized: Please, do NOT normalize the vertices
			3 * 4,	//stride: 3points * 4bytes
			0);		//offset: The byte position of the first vertex in the buffer is 0.
		drawArrays(TRIANGLE_STRIP, 0, static_cast<int>(_squareVertices.size()) / 3);
	}

	void GameWidget::handleKeyStateChanged(Wt::Key key, bool newState)
	{
		/*WApplication *app = WApplication::instance();
		if(!app->keyState().anyEnabled())
		{
			_timer->stop();
		}
		else if(!_timer->isActive())
		{
			_timer->start();
			handleTimer();
		}*/
	}
	void GameWidget::handleTimer()
	{
		/*WApplication *app = WApplication::instance();

		if(app->keyState().arrowUp)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Top).value() - 2), Wt::Top);
		if(app->keyState().arrowDown)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Top).value() + 2), Wt::Top);
		if(app->keyState().arrowLeft)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Left).value() - 2), Wt::Left);
		if(app->keyState().arrowRight)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Left).value() + 2), Wt::Left);*/
	}
}