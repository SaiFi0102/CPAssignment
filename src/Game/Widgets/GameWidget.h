#ifndef DB_GAME_WIDGET_H
#define DB_GAME_WIDGET_H

#include <Wt/WGlobal>
#include <Wt/WGLWidget>

namespace DB
{
	class GameWidget : public Wt::WGLWidget
	{
	public:
		GameWidget(Wt::WContainerWidget *parent = nullptr);
		//virtual ~GameWidget();

		virtual void initializeGL() override;
		virtual void paintGL() override;
		virtual void resizeGL(int width, int height) override;

	protected:
		void handleKeyStateChanged(Wt::Key key, bool newState);
		void handleTimer();

		bool _initialized = false;
		Wt::WText *_test = nullptr;

		//Program and related variables
		Program _shaderProgram;
		AttribLocation _vertexPositionAttribute;
		UniformLocation _pMatrixUniform;
		UniformLocation _mvMatrixUniform;
		UniformLocation _cMatrixUniform;

		//A client-side JavaScript matrix variable
		JavaScriptMatrix4x4 _jsMvMatrix;
		JavaScriptVector _jsTranslateTriangleVector;
		JavaScriptVector _jsTranslateSquareVector;

		//VBOs, Vertex Buffer Objects, contains vertexes
		static const std::vector<float> _triangleVertices;
		static const std::vector<float> _squareVertices;

		Buffer _triangleVertexPositionBuffer;
		Buffer _squareVertexPositionBuffer;
	};
}

#endif
