#pragma once

class App
{

public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void RunFrame();
	/*void Update( float deltaSeconds );*/

	bool IsQuitting() const { return m_isQuitting; }
	bool HandleQuitRequested();

	void RestartGame();

	static void Command_Close();

private:
	void BeginFrame();
	void Render() const;
	void EndFrame();

public:
	bool m_isQuitting		   = false;
	bool g_isDebugMode		   = false;
	bool m_isPaused			   = false;
	bool m_isIgnoreSolidModeOn = false;
	bool m_isWholeMapModeOn    = false;
	bool m_isSlowModeOn        = false;
	bool m_isFourSpeedModeOn   = false;
};