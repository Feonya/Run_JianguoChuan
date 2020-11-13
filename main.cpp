#include <clocale>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <ncursesw/ncurses.h>

/* Ncurses窗口位置和大小 */
#define WIN_X 0
#define WIN_Y 0
#define WIN_W 124
#define WIN_H 81

/* 边框的位置和大小 */
#define BDR_X 0
#define BDR_Y 0
#define BDR_W WIN_W - 1
#define BDR_H WIN_H - 1

/* 游戏区域位置和大小 */
#define PGR_X BDR_X + 1
#define PGR_Y BDR_Y + 1
#define PGR_W BDR_W - 1
#define PGR_H BDR_H - 1

/* 地面的水平位置 */
#define FLR_Y PGR_Y + PGR_H - 8

WINDOW* g_win;  // 指向我们的ncurses窗口

// 初始化ncurses库
void initNCurses() {
    setlocale(LC_ALL, "");  // 设置语言环境为操作系统默认
    srand(time(nullptr));   // 以当前时间为种子初始化伪随机数生成器

    initscr();  // ncurses初始化库函数
    cbreak();   // 禁用行缓冲，让每个输入字符立即可用。否则输入字符串将以行缓冲，直到按下回车键才可被程序使用
    noecho();   // 光标处不打印输入字符
    start_color();  // 使用彩色支持，如果你的终端支持彩色的话
    curs_set(0);    // 隐藏光标

    // 创建新ncurses窗口，参数分别为末尾行数、末尾列数、起始行数、起始列数
    g_win = newwin(WIN_H + WIN_Y, WIN_W + WIN_X, WIN_Y, WIN_X);
    nodelay(g_win, true);  // 让getch()之类的字符获取函数不阻塞进程
    keypad(g_win, true);   // 开启小键盘按键及方向键支持

    /* 将三个色彩对注册到它们各自的索引数字上 */
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED,    COLOR_BLACK);
    init_pair(3, COLOR_GREEN,  COLOR_BLACK);
}

bool g_isRunning = true;  // 游戏是否运行
int  g_ticks     = 0;     // 游戏开始到当前帧的计数
int  g_score     = 0;     // 游戏得分

float g_graAcc = 0.2f;  // 重力加速度

// 角色结构体
struct Kid {
    int x, y, w, h;  // 水平位置、垂直位置、宽、高
    float vx;        // 水平速率。关于速率和速度的区别：速率有方向，速度无方向
    float vy;        // 垂直速率
    int jumpSpd;     // 跳跃的瞬时速度
    int moveSpd;     // 移动速度

    std::vector<std::string> appearances;  // 存储本角色的所有形象字符串
    int apprNum;      // 本角色有几个不同形象
    int currApprIdx;  // 所有形象中，当前形象的索引

    bool isMyNameCalledChuanJianguo;  // 本角色的名字是否叫川建国
    bool canJump;  // 标识角色是否可以跳跃，用于生产跳跃和不跳跃的敌人
};

// 定义川建国
Kid g_trump = { 20, FLR_Y - 21, 25, 21, 0.0f, 0.0f, 4, 3, { R"(          ___ _________ ___ )""\n"
                                                            R"(        _/  /          \\  \)""\n"
                                                            R"(       /                   |)""\n"
                                                            R"(      /                 __/ )""\n"
                                                            R"(      |       ___\_||/_/    )""\n"
                                                            R"(      |    __/      __ \    )""\n"
                                                            R"(      |   /  \      <O |    )""\n"
                                                            R"(     /    |        =   \    )""\n"
                                                            R"(     |   / \_          _|   )""\n"
                                                            R"(     |  |            _|     )""\n"
                                                            R"(      \_|____|        \     )""\n"
                                                            R"(         |____\_______/     )""\n"
                                                            R"(        /\__/ \__/\         )""\n"
                                                            R"(       /   /\_/\   \        )""\n"
                                                            R"(      / | /  |  \ | \       )""\n"
                                                            R"(    _/_/| |  |  | |\_\_     )""\n"
                                                            R"(   /__/ | | / \ | | \__\    )""\n"
                                                            R"(        / |/   \| \         )""\n"
                                                            R"(       /___________\        )""\n"
                                                            R"(           |__||_           )""\n"
                                                            R"(           |____\\          )""\n",
                                                            R"(          ___ _________ ___ )""\n"
                                                            R"(        _/  /          \\  \)""\n"
                                                            R"(       /                   |)""\n"
                                                            R"(      /                 __/ )""\n"
                                                            R"(      |       ___\_||/_/    )""\n"
                                                            R"(      |    __/      __ \    )""\n"
                                                            R"(      |   /  \      <O |    )""\n"
                                                            R"(     /    |        =   \    )""\n"
                                                            R"(     |   / \_          _|   )""\n"
                                                            R"(     |  |            _|     )""\n"
                                                            R"(      \_|____|        \     )""\n"
                                                            R"(        _|____\_______/     )""\n"
                                                            R"(       / \__/ \__/ \        )""\n"
                                                            R"(      / |  /\_/\  | \       )""\n"
                                                            R"(    _/_/| /  |  \ |\_\_     )""\n"
                                                            R"(   /__/ | |  |  | | \__\    )""\n"
                                                            R"(        | | / \ | |         )""\n"
                                                            R"(        / |/   \| \         )""\n"
                                                            R"(       /___________\        )""\n"
                                                            R"(        |__|_  |__|_        )""\n"
                                                            R"(        |____\ |____\       )""\n" }, 2, 0, true, true };

// 定义一个川建国的敌人，因为游戏每次只需要出现一个敌人，因此我们只定义这一个
Kid g_cat = { WIN_W, FLR_Y - 9, 10, 9, -1.0f, 0.0f, 4, 1, { R"(|\      /|          _)""\n"
                                                            R"(|-\____/-|         //)""\n"
                                                            R"(|        |        // )""\n"
                                                            R"(|  O O   |_______//  )""\n"
                                                            R"( \__^___/         \  )""\n"
                                                            R"(   |              |  )""\n"
                                                            R"(   / __  ______   \  )""\n"
                                                            R"(  / /  \ \    / /\ \ )""\n"
                                                            R"( /_/    \_\  /_/  \_\)""\n",
                                                            R"(|\      /|     _     )""\n"
                                                            R"(|-\____/-|     \\    )""\n"
                                                            R"(|        |      \\   )""\n"
                                                            R"(|  O O   |_______\\  )""\n"
                                                            R"( \__^___/         \  )""\n"
                                                            R"(   |              |  )""\n"
                                                            R"(   | __  _____ __ |  )""\n"
                                                            R"(   | || |    | || |  )""\n"
                                                            R"(   |_||_|    |_||_|  )""\n" }, 2, 0, false, false };

// 执行可以跳跃的敌人的跳跃的
void handleCatJump() {
    if (g_cat.canJump && g_cat.y >= FLR_Y - g_cat.h)
        g_cat.vy = -g_cat.jumpSpd;
}

// 生成新敌人
void checkAndGenerateNewCat() {
    if (g_cat.x < WIN_X - g_cat.w) {  // 如果敌人移动超出窗口左侧
        int rndNum    = rand() % WIN_W;  // 生成一个随机数
        g_cat.x       = WIN_W +rndNum;   // 根据这个随机数将这个敌人放到窗口右侧的一个随即范围内
        g_cat.canJump = rndNum < 10 ? true : false;  // 根据这个随机数设置敌人是否可以跳跃
        g_cat.vx     -= 0.1f;            // 每次新生成敌人，向左的速率增加一点
        g_score++;                       // 得分加1
    }
}

// 根据ticks切换角色的形象索引
void playKidAnim(Kid& f_k) {
    if (g_ticks % 4 == 0)
        f_k.currApprIdx = ++f_k.currApprIdx < f_k.apprNum ? f_k.currApprIdx : 0;
}

// 移动角色
void moveKid(Kid& f_k) {
    f_k.x += f_k.vx;  // 水平移动角色
    // 如果他是川建国限制其水平移动范围
    if (f_k.isMyNameCalledChuanJianguo)
        f_k.x = f_k.x <= PGR_X ? PGR_X : f_k.x > PGR_W + PGR_X - f_k.w ? PGR_W + PGR_X - f_k.w : f_k.x;

    f_k.y += f_k.vy;  // 计算垂直位移
    if (f_k.y > FLR_Y - f_k.h) f_k.y   = FLR_Y - f_k.h;  // 当角色脚部低于地面时，角色脚部置于地面上
    if (f_k.y < FLR_Y - f_k.h) f_k.vy += g_graAcc;       // 当角色脚部离开地面时，垂直方向上计算重力加速度影响
}

// 绘制角色，注意原始形象字符串除首尾行外，需要根据换行符和角色水平位置进行字符串行位置修正
void drawKid(Kid& f_k) {
    int cx = f_k.x;  // 用于标识当前形象字符的列坐标位置，初始列坐标位置即角色的水平坐标位置
    int ly = f_k.y;  // 用于标识当前形象行字符串（以换行符分割）的行坐标位置，初始行坐标位置即角色的垂直坐标位置
    // 遍历形象字符串中的所有字符，注意字符串数组最后一位是\0，标识字符串结束，我们不想打印它
    for (unsigned i = 0; i < f_k.appearances[f_k.currApprIdx].size() - 1; ++i) {
        if (f_k.appearances[f_k.currApprIdx][i] == '\n') {  // 若当前字符为换行符时
            cx = f_k.x;  // 列坐标位置标识还原到角色的水平坐标位置
            ly++;        // 行坐标位置标识下移一行
            continue;    // 跳过后续代码，进行下一轮循环，即不打印换行符
        } else if (f_k.appearances[f_k.currApprIdx][i] == ' ') {  // 若当前字符为空格时
            cx++;      // 列坐标位置标识后移一位
            continue;  // 跳过后续代码，进行下一轮循环，即不打印换行符
        }
        // 根据当前字符列坐标位置和当前行字符串行坐标位置打印当前字符
        mvwaddch(g_win, ly, cx++, f_k.appearances[f_k.currApprIdx][i]);
    }
}

// 碰撞检测
void checkCollision() {
    if (g_cat.x + g_cat.w    >= g_trump.x && g_cat.x <= g_trump.x + g_trump.w - 1 &&
        g_cat.y + g_cat.h - 1>= g_trump.y && g_cat.y <= g_trump.y + g_trump.h - 1) {
        g_isRunning = false;
    }
}

// 获取输入
void input() {
    int ch = tolower(wgetch(g_win));  // 为当前窗口获取输入字符，并将该字符转小写
    switch (ch) {
        // 按下q键时，将g_isRunnings设为false，本帧结束后游戏退出
        case 'q':
            g_isRunning = false;
            break;
        // 按下w键时，川建国垂直速度设为向上的瞬时速度
        case 'w':
            if (g_trump.y >= FLR_Y - g_trump.h)  // 根据川建国是否在地面做条件限制
                g_trump.vy = -g_trump.jumpSpd;
            break;
        // 按下a键时，川建国水平速度向左
        case 'a':
            if (g_trump.x > PGR_X)  // 根据川建国是否在游戏区域内做条件限制
                g_trump.vx = -g_trump.moveSpd;
            break;
        // 按下d键时，川建国水平速度向右
        case 'd':
            if (g_trump.x < PGR_X + PGR_W - g_trump.w)  // 根据川建国是否在游戏区域内做条件限制
                g_trump.vx = g_trump.moveSpd;
            break;
        // 没有按键按下时，川建国水平速度为0
        default:
            g_trump.vx = 0.0f;
            break;
    }
}

// 更新游戏数据
void update() {
    handleCatJump();

    playKidAnim(g_cat);
    moveKid(g_cat);
    playKidAnim(g_trump);
    moveKid(g_trump);

    checkAndGenerateNewCat();
    checkCollision();
}

// 渲染游戏画面
void render() {
    werase(g_win);  // 立即清空屏幕

    mvwhline(g_win, FLR_Y - 1, PGR_X + 1, '_', PGR_W - 2);  // 绘制地面

    /* 如果川建国的水平位置小于敌人的水平位置，则先绘制敌人，否则先绘制川建国 */
    if (g_trump.x < g_cat.x) {
        drawKid(g_cat);
        drawKid(g_trump);
    } else {
        drawKid(g_trump);
        drawKid(g_cat);
    }

    // 绘制边框、边框文字及得分
    wattron(g_win, A_BOLD);
    box(g_win, 0, 0);
    wattron(g_win, COLOR_PAIR(2));
    mvwaddstr(g_win, BDR_Y, 48, "[ RUN! Jianguo Chuan! ]");
    wattroff(g_win, COLOR_PAIR(2));
    wattron(g_win, COLOR_PAIR(1));
    mvwaddstr(g_win, BDR_H, 90, "[ Developed by 2D_Cat ]");
    wattroff(g_win, COLOR_PAIR(1));
    wattron(g_win, COLOR_PAIR(3));
    mvwprintw(g_win, FLR_Y + 4, 60, "Score: %d", g_score);
    wattroff(g_win, COLOR_PAIR(3));
    wattroff(g_win, A_BOLD);

    wrefresh(g_win);  // 刷新屏幕
}

// 推出游戏
void quit() {
    delwin(g_win);
    endwin();

    std::cout << "\n";
    std::cout << "*********************************************\n";
    std::cout << "\n";
    std::cout << "                Game Over!\n";
    std::cout << "\n";
    std::cout << "                Your score: " << g_score << "\n";
    std::cout << "\n";
    std::cout << "*********************************************\n";
    std::cout << "\n";
}

int main() {
    initNCurses();

    while (g_isRunning) {
        input();
        update();
        render();

        g_ticks++;
        Sleep(20);  // 休眠20ms
    }

    quit();

    return 0;
}
