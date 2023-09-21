#include <windows.h>
#include <filter.h>
#include <exedit.hpp>

#define DLG_SCENE_JMP 0x1001

FILTER_DLL filter = {
    FILTER_FLAG_ALWAYS_ACTIVE,
    NULL,NULL,
    const_cast<char*>("ÉVÅ[ÉìÉWÉÉÉìÉv"),
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    func_WndProc,
};
EXTERN_C FILTER_DLL __declspec(dllexport)* __stdcall GetFilterTable(void) {
    return &filter;
}
FILTER* exeditfp;

FILTER* get_exeditfp(FILTER* fp) {
    SYS_INFO si;
    fp->exfunc->get_sys_info(NULL, &si);

    for (int i = 0; i < si.filter_n; i++) {
        FILTER* tfp = (FILTER*)fp->exfunc->get_filterp(i);
        if (tfp->information != NULL) {
            if (!strcmp(tfp->information, "ägí£ï“èW(exedit) version 0.92 by ÇjÇdÇmÇ≠ÇÒ")) return tfp;
        }
    }
    return NULL;
}

ExEdit::Object** ObjectArray_ptr; // 0x1e0fa4
int* SettingDialogObjectIndex_ptr; // 0x177a10
int* CurrentSceneIndex_ptr; // 0x1a5310
ExEdit::SceneSetting* SceneSettingTable; // 0x177a50
ExEdit::Filter** ExEditFilterTable; // 0x187c98
ExEdit::Filter* SceneFilter_ptr; // 0xaea10
ExEdit::Filter* SceneAudioFilter_ptr; // 0xaebc8

static inline void(__cdecl* change_disp_scene)(int scene_idx, ExEdit::Filter* efp, void* editp); // 2ba60
static inline void*(__cdecl* get_exdata_ptr)(ExEdit::ObjectFilterIndex ofi); // 47b40
static inline ExEdit::ObjectFilterIndex(__cdecl* search_above_object)(ExEdit::ObjectFilterIndex ofi, char* name); // 4b300

void scene_jmp(void* editp) {
    int idx = *SettingDialogObjectIndex_ptr;
    if (idx < 0) return;

    auto obj = *ObjectArray_ptr + idx;
    if (((byte)obj->flag & 1) == 0) return;

    auto efp = ExEditFilterTable[obj->filter_param->id];
    if (efp == SceneAudioFilter_ptr) {
        if (obj->check_value[obj->filter_param->check_begin + 1]) {
            idx = (int)search_above_object((ExEdit::ObjectFilterIndex)(idx + 1), (char*)SceneFilter_ptr->name);
            if (idx & 0xffff0000) return;

            idx--;
            if (idx < 0) return;

            obj = *ObjectArray_ptr + idx;
            if (((byte)obj->flag & 1) == 0) return;

            efp = ExEditFilterTable[obj->filter_param->id];
        }
    }
    if (efp != SceneFilter_ptr && efp != SceneAudioFilter_ptr) return;

    int scene_idx = *(int*)get_exdata_ptr((ExEdit::ObjectFilterIndex)(idx + 1));
    if (scene_idx < 1 || 49 < scene_idx || scene_idx == *CurrentSceneIndex_ptr) return;

    auto scene = SceneSettingTable + scene_idx;
    int frame = min(obj->track_value_left[obj->filter_param->track_begin] - 1, scene->max_frame);
    scene->timeline_disp_begin_pos = scene->current_frame = frame;
    
    change_disp_scene(scene_idx, efp, editp);

}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, FILTER* fp) {
    switch (message) {
    case WM_FILTER_INIT:
        exeditfp = get_exeditfp(fp);
        if (exeditfp == NULL) {
            MessageBoxA(fp->hwnd, "ägí£ï“èW0.92Ç™å©Ç¬Ç©ÇËÇ‹ÇπÇÒÇ≈ÇµÇΩ", fp->name, MB_OK);
            break;
        }
        fp->exfunc->add_menu_item(fp, fp->name, fp->hwnd, DLG_SCENE_JMP, NULL, NULL);


        ObjectArray_ptr = (ExEdit::Object**)((int)exeditfp->dll_hinst + 0x1e0fa4);
        SettingDialogObjectIndex_ptr = (int*)((int)exeditfp->dll_hinst + 0x177a10);
        CurrentSceneIndex_ptr = (int*)((int)exeditfp->dll_hinst + 0x1a5310);
        SceneSettingTable = (ExEdit::SceneSetting*)((int)exeditfp->dll_hinst + 0x177a50);
        ExEditFilterTable = (ExEdit::Filter**)((int)exeditfp->dll_hinst + 0x187c98);
        SceneFilter_ptr = (ExEdit::Filter*)((int)exeditfp->dll_hinst + 0xaea10);
        SceneAudioFilter_ptr = (ExEdit::Filter*)((int)exeditfp->dll_hinst + 0xaebc8);

        change_disp_scene = reinterpret_cast<decltype(change_disp_scene)>((int)exeditfp->dll_hinst + 0x2ba60);
        get_exdata_ptr = reinterpret_cast<decltype(get_exdata_ptr)>((int)exeditfp->dll_hinst + 0x47b40);
        search_above_object = reinterpret_cast<decltype(search_above_object)>((int)exeditfp->dll_hinst + 0x4b300);



        break;
    case WM_FILTER_COMMAND:
        switch (wparam) {
        case DLG_SCENE_JMP:
            scene_jmp(editp);
            break;
        }
    }
    return FALSE;
}
