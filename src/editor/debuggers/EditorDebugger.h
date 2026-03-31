#pragma once

#include "editor/debuggers/IDebugTab.h"

class Editor;

class EditorDebugger : public IDebugTab
{
private:
    Editor* m_editorPtr = nullptr;

public:
    EditorDebugger();

    void RegisterEditor(Editor* editor);
    void UnRegisterEditor();

    void DrawTab(DebugTabContext& context);
};
