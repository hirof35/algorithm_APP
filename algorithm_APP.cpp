#include "DxLib.h"
#include <vector>
#include <algorithm>

// --- 設定・列挙型 ---
enum Scene { SCENE_MENU, SCENE_COLLISION, SCENE_STACK, SCENE_SEARCH, SCENE_QUEUE, SCENE_MERGE_SORT };
Scene currentScene = SCENE_MENU;

// --- グローバルデータ ---
int data[16];           // ソート用データ
std::vector<int> stackData;
std::vector<int> queueData;
float queueX[8];        // キューのアニメーション用座標

// --- ボタン・UI共通関数 ---
bool DrawButton(int x, int y, int w, int h, const char* label, unsigned int color) {
    int mx, my; GetMousePoint(&mx, &my);
    bool hover = (mx >= x && mx <= x + w && my >= y && my <= y + h);
    unsigned int drawCol = hover ? color + 0x333333 : color;
    DrawBox(x, y, x + w, y + h, drawCol, TRUE);
    DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), FALSE);
    DrawFormatString(x + 10, y + h / 2 - 8, GetColor(255, 255, 255), label);
    return hover && (GetMouseInput() & MOUSE_INPUT_LEFT);
}

void DrawMemo(const char* title, const char* text) {
    DrawBox(380, 320, 620, 460, GetColor(40, 40, 40), TRUE);
    DrawBox(380, 320, 620, 460, GetColor(200, 200, 200), FALSE);
    DrawFormatString(390, 330, GetColor(255, 255, 0), "【%s】", title);
    DrawFormatString(390, 360, GetColor(255, 255, 255), text);
}

// ==========================================
// 1. 当たり判定（Collision）
// ==========================================
void Room_Collision() {
    static int cx = 100, cy = 100, cr = 30;
    static int rx = 300, ry = 200, rw = 120, rh = 80;
    GetMousePoint(&cx, &cy);
    bool hit = (cx > rx && cx < rx + rw && cy > ry && cy < ry + rh);
    DrawBox(rx, ry, rx + rw, ry + rh, hit ? GetColor(255, 0, 0) : GetColor(0, 255, 0), FALSE);
    DrawCircle(cx, cy, cr, GetColor(0, 200, 255), TRUE);
    DrawMemo("COLLISION", "マウスの円と四角の\n当たり判定です。\n座標の重なりをif文で\nチェックしています。");
}

// ==========================================
// 2. スタック（Stack）
// ==========================================
void Room_Stack() {
    if (DrawButton(450, 100, 100, 40, "PUSH", GetColor(0, 100, 200))) {
        if (stackData.size() < 8) { stackData.push_back(GetRand(99)); WaitTimer(150); }
    }
    if (DrawButton(450, 150, 100, 40, "POP", GetColor(200, 100, 0))) {
        if (!stackData.empty()) { stackData.pop_back(); WaitTimer(150); }
    }
    DrawBox(200, 100, 300, 450, GetColor(255, 255, 255), FALSE);
    for (int i = 0; i < (int)stackData.size(); i++) {
        int y = 410 - (i * 40);
        DrawBox(210, y, 290, y + 35, GetColor(0, 150, 255), TRUE);
        DrawFormatString(240, y + 10, GetColor(255, 255, 255), "%d", stackData[i]);
    }
    DrawMemo("STACK", "LIFO (後入れ先出し)\n新しいデータが上に積み\n重なり、上から順に\n取り出されます。");
}

// ==========================================
// 3. 探索競争（Search）
// ==========================================
void Room_Search() {
    static int sData[32]; static bool init = false;
    static int lIdx = -1, bL = 0, bR = 31, bM = -1;
    if (!init) { for (int i = 0; i < 32; i++) sData[i] = i * 3; init = true; }
    if (DrawButton(250, 400, 140, 40, "START", GetColor(100, 100, 0))) { lIdx = 0; bL = 0; bR = 31; }
    if (lIdx >= 0 && GetNowCount() % 10 == 0) {
        if (lIdx < 31) lIdx++;
        if (bL <= bR) { bM = (bL + bR) / 2; if (sData[bM] < 75) bL = bM + 1; else if (sData[bM] > 75) bR = bM - 1; }
    }
    for (int i = 0; i < 32; i++) {
        DrawBox(20 + i * 19, 100, 35 + i * 19, 180, (i == lIdx) ? GetColor(255, 255, 0) : GetColor(50, 50, 50), TRUE);
        unsigned int bCol = (i >= bL && i <= bR) ? GetColor(0, 255, 0) : GetColor(30, 30, 30);
        if (i == bM) bCol = GetColor(255, 0, 0);
        DrawBox(20 + i * 19, 220, 35 + i * 19, 300, bCol, TRUE);
    }
    DrawMemo("SEARCH", "上：線形探索\n下：二分探索\n二分探索が圧倒的に速く\n見つける様子に注目！");
}

// ==========================================
// 4. キュー（Queue）
// ==========================================
void Room_Queue() {
    if (DrawButton(450, 100, 100, 40, "ENQUEUE", GetColor(0, 150, 100))) {
        if (queueData.size() < 8) { queueData.push_back(GetRand(99)); WaitTimer(150); }
    }
    if (DrawButton(450, 150, 100, 40, "DEQUEUE", GetColor(200, 50, 50))) {
        if (!queueData.empty()) { queueData.erase(queueData.begin()); WaitTimer(150); }
    }
    for (int i = 0; i < (int)queueData.size(); i++) {
        float tx = 50.0f + (i * 60.0f);
        queueX[i] += (tx - queueX[i]) * 0.1f;
        DrawBox((int)queueX[i], 250, (int)queueX[i] + 50, 300, GetColor(0, 150, 255), TRUE);
        DrawFormatString((int)queueX[i] + 15, 265, GetColor(255, 255, 255), "%d", queueData[i]);
    }
    DrawMemo("QUEUE", "FIFO (先入れ先出し)\n行列のように先に並んだ\nものから順に出ていく\n構造です。");
}

// ==========================================
// 5. マージソート（Merge Sort）
// ==========================================
void DrawMergeArray(int l, int r) {
    ClearDrawScreen();
    DrawButton(10, 10, 80, 30, "BACK", GetColor(100, 100, 100)); // 描画維持用
    for (int i = 0; i < 16; i++) {
        unsigned int col = (i >= l && i <= r) ? GetColor(255, 255, 0) : GetColor(100, 100, 100);
        DrawBox(50 + i * 35, 350 - data[i] * 3, 80 + i * 35, 350, col, TRUE);
    }
    DrawMemo("MERGE SORT", "分割して整列し、最後に\n合体させるソートです。\n再帰呼び出しによって\n動いています。");
    ScreenFlip();
}

void Merge(int l, int m, int r) {
    int temp[16], i = l, j = m + 1, k = 0;
    while (i <= m && j <= r) { temp[k++] = (data[i] <= data[j]) ? data[i++] : data[j++]; DrawMergeArray(l, r); WaitTimer(50); }
    while (i <= m) temp[k++] = data[i++];
    while (j <= r) temp[k++] = data[j++];
    for (i = 0; i < k; i++) { data[l + i] = temp[i]; DrawMergeArray(l, r); WaitTimer(50); }
}

void MergeSort(int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        MergeSort(l, m); MergeSort(m + 1, r);
        Merge(l, m, r);
    }
}

void Room_MergeSort() {
    static bool init = false;
    if (!init) { for (int i = 0; i < 16; i++) data[i] = GetRand(80) + 10; init = true; }
    DrawMergeArray(-1, -1);
    if (DrawButton(250, 400, 140, 40, "SORT START", GetColor(150, 0, 150))) { MergeSort(0, 15); }
    if (DrawButton(400, 400, 100, 40, "SHUFFLE", GetColor(100, 100, 100))) { init = false; }
}

// --- メイン ---
int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR l, int n) {
    ChangeWindowMode(TRUE); if (DxLib_Init() == -1) return -1; SetDrawScreen(DX_SCREEN_BACK);
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
        ClearDrawScreen();
        if (currentScene == SCENE_MENU) {
            DrawString(200, 50, "=== ALGORITHM VISUALIZER ===", GetColor(255, 255, 255));
            if (DrawButton(220, 120, 200, 45, "1. COLLISION", GetColor(100, 50, 50))) currentScene = SCENE_COLLISION;
            if (DrawButton(220, 180, 200, 45, "2. STACK", GetColor(50, 100, 50))) currentScene = SCENE_STACK;
            if (DrawButton(220, 240, 200, 45, "3. SEARCH RACE", GetColor(50, 50, 100))) currentScene = SCENE_SEARCH;
            if (DrawButton(220, 300, 200, 45, "4. QUEUE", GetColor(25, 70, 70))) currentScene = SCENE_QUEUE;
            if (DrawButton(220, 360, 200, 45, "5. MERGE SORT", GetColor(80, 30, 80))) currentScene = SCENE_MERGE_SORT;
        }
        else {
            switch (currentScene) {
            case SCENE_COLLISION: Room_Collision(); break;
            case SCENE_STACK:     Room_Stack(); break;
            case SCENE_SEARCH:    Room_Search(); break;
            case SCENE_QUEUE:     Room_Queue(); break;
            case SCENE_MERGE_SORT:Room_MergeSort(); break;
            }
            if (DrawButton(10, 10, 80, 30, "BACK", GetColor(100, 100, 100))) currentScene = SCENE_MENU;
        }
        ScreenFlip();
    }
    DxLib_End(); return 0;
}
