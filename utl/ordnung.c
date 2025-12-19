#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <direct.h>

/* --- Configuration --- */
#define MAX_ITEMS 2048
#define MAX_PATH_LEN 512
#define CONFIG_FILENAME "folders.cfg"
#define UI_HEIGHT 20

/* --- Key Codes --- */
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACKSPACE 8

/* --- Modes --- */
#define MODE_FAVORITES 0
#define MODE_BROWSE 1
#define MODE_DRIVES 2

/* --- Data Structures --- */
struct ListItem {
    char label[MAX_PATH_LEN];
    char path[MAX_PATH_LEN];
    int is_dir;
};

/* FIX: Declared globally to avoid Stack Overflow. 
   (Previous version tried to put ~2MB on a 1MB stack)
*/
static struct ListItem items[MAX_ITEMS];

/* --- Helpers --- */

static void set_cursor_pos(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

static void hide_cursor(void) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

static void clear_screen(void) {
    HANDLE hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hStdOut, (TCHAR) ' ', cellCount, homeCoords, &count)) return;
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count)) return;
    SetConsoleCursorPosition(hStdOut, homeCoords);
}

/* --- Logic --- */

static void load_config(int *count) {
    FILE *fp;
    char line[MAX_PATH_LEN * 2];
    char *token;

    *count = 0;
    fp = fopen(CONFIG_FILENAME, "r");
    if (!fp) {
        /* Generate default config if missing */
        fp = fopen(CONFIG_FILENAME, "w");
        if (fp) {
            fprintf(fp, "Documents|C:\\Users\\Public\\Documents\n");
            fprintf(fp, "Windows|C:\\Windows\n");
            fprintf(fp, "Program Files|C:\\Program Files\n");
            fclose(fp);
        }
        strcpy(items[0].label, "Documents");
        strcpy(items[0].path, "C:\\Users\\Public\\Documents");
        items[0].is_dir = 1;
        
        strcpy(items[1].label, "Windows");
        strcpy(items[1].path, "C:\\Windows");
        items[1].is_dir = 1;

        strcpy(items[2].label, "Program Files");
        strcpy(items[2].path, "C:\\Program Files");
        items[2].is_dir = 1;

        *count = 3;
        return;
    }

    while (fgets(line, sizeof(line), fp) && *count < MAX_ITEMS) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        token = strtok(line, "|");
        if (token) {
            strncpy(items[*count].label, token, MAX_PATH_LEN - 1);
            items[*count].label[MAX_PATH_LEN - 1] = 0;
            
            token = strtok(NULL, "|");
            if (token) {
                strncpy(items[*count].path, token, MAX_PATH_LEN - 1);
                items[*count].path[MAX_PATH_LEN - 1] = 0;
            } else {
                strcpy(items[*count].path, items[*count].label);
            }
            items[*count].is_dir = 1;
            (*count)++;
        }
    }
    fclose(fp);
}

static void scan_drives(int *count) {
    DWORD drives;
    char drive_str[4];
    int i;
    
    *count = 0;
    drives = GetLogicalDrives();
    
    for (i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            drive_str[0] = (char)('A' + i);
            drive_str[1] = ':';
            drive_str[2] = '\\';
            drive_str[3] = '\0';
            
            sprintf(items[*count].label, "Drive (%s)", drive_str);
            strcpy(items[*count].path, drive_str);
            items[*count].is_dir = 1;
            (*count)++;
        }
    }
}

static void scan_directory(const char *path, int *count) {
    char search_path[MAX_PATH_LEN];
    WIN32_FIND_DATA find_data;
    HANDLE h_find;

    *count = 0;

    /* Add "Up one level" entry */
    strcpy(items[*count].label, "..");
    strcpy(items[*count].path, "..");
    items[*count].is_dir = 1;
    (*count)++;

    sprintf(search_path, "%s\\*", path);
    h_find = FindFirstFile(search_path, &find_data);

    if (h_find != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                continue;
            }
            if (*count < MAX_ITEMS) {
                strncpy(items[*count].label, find_data.cFileName, MAX_PATH_LEN - 1);
                items[*count].label[MAX_PATH_LEN - 1] = 0;
                
                /* Safe formatting for path */
                if (strlen(path) + strlen(find_data.cFileName) + 2 < MAX_PATH_LEN) {
                    sprintf(items[*count].path, "%s\\%s", path, find_data.cFileName);
                } else {
                    continue; /* Skip if path too long */
                }
                
                items[*count].is_dir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
                (*count)++;
            }
        } while (FindNextFile(h_find, &find_data) != 0);
        FindClose(h_find);
    }
}

static void go_up_directory(char *path) {
    char *last_slash = strrchr(path, '\\');
    /* Handle roots like C:\ correctly */
    if (last_slash && last_slash != path) {
        if (*(last_slash - 1) != ':') {
             *last_slash = '\0';
        } else {
             /* If C:\, ensure we don't remove the backslash if it's the root */
             *(last_slash + 1) = '\0'; 
        }
    }
}

static void draw_ui(int count, int selected, int mode, const char *current_path) {
    int i;
    int scroll_offset = 0;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    /* Colors */
    WORD headerColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    WORD selectColor = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    WORD fileColor   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD dirColor    = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Yellow-ish
    
    if (selected >= UI_HEIGHT) {
        scroll_offset = selected - UI_HEIGHT + 1;
    }

    set_cursor_pos(0, 0);
    SetConsoleTextAttribute(hConsole, headerColor);
    printf("============================================================\n");
    
    if (mode == MODE_FAVORITES) {
        printf(" FAVORITES                                                  \n");
    } else if (mode == MODE_DRIVES) {
        printf(" SELECT DRIVE                                               \n");
    } else {
        char display_path[60];
        int len = strlen(current_path);
        if (len > 55) {
            sprintf(display_path, "...%s", current_path + len - 52);
            printf(" %-58s \n", display_path);
        } else {
            printf(" %-58s \n", current_path);
        }
    }
    printf("============================================================\n");
    
    for (i = 0; i < UI_HEIGHT; i++) {
        int idx = i + scroll_offset;
        set_cursor_pos(0, 3 + i);
        
        if (idx < count) {
            if (idx == selected) {
                SetConsoleTextAttribute(hConsole, selectColor);
                if (items[idx].is_dir)
                    printf(" -> [%-50s] \n", items[idx].label);
                else
                    printf(" ->  %-52s \n", items[idx].label);
            } else {
                if (items[idx].is_dir) {
                    SetConsoleTextAttribute(hConsole, dirColor);
                    printf("    [%-50s] \n", items[idx].label);
                } else {
                    SetConsoleTextAttribute(hConsole, fileColor);
                    printf("     %-52s \n", items[idx].label);
                }
            }
        } else {
            /* Clear empty lines */
            SetConsoleTextAttribute(hConsole, fileColor);
            printf("                                                            \n");
        }
    }

    set_cursor_pos(0, 3 + UI_HEIGHT);
    SetConsoleTextAttribute(hConsole, headerColor);
    printf("============================================================\n");
    SetConsoleTextAttribute(hConsole, fileColor);
    
    if (mode == MODE_FAVORITES) {
        printf(" [ENTER] Browse  [S+ENTER] Explorer  [D] Drives  [Q] Quit   \n");
    } else if (mode == MODE_DRIVES) {
         printf(" [ENTER] Select  [ESC] Cancel                               \n");
    } else {
        printf(" [ENTER] Open    [S+ENTER] Explorer  [BS] Up     [ESC] Favs \n");
    }
}

int main(void) {
    int count = 0;
    int selected = 0;
    int ch;
    int running = 1;
    int mode = MODE_FAVORITES;
    int prev_mode = MODE_FAVORITES;
    char current_path[MAX_PATH_LEN] = {0};

    clear_screen();
    load_config(&count); /* Uses global 'items' */
    hide_cursor();

    while (running) {
        draw_ui(count, selected, mode, current_path);

        ch = _getch();
        if (ch == 0 || ch == 224) {
            ch = _getch();
            switch (ch) {
                case KEY_UP:
                    if (selected > 0) selected--;
                    else if (count > 0) selected = count - 1;
                    break;
                case KEY_DOWN:
                    if (selected < count - 1) selected++;
                    else selected = 0;
                    break;
            }
        } else {
            switch (ch) {
                case KEY_ENTER:
                    /* Check for SHIFT key for Explorer Open */
                    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && mode != MODE_DRIVES) {
                        if (strcmp(items[selected].label, "..") == 0) {
                             ShellExecute(NULL, "open", current_path, NULL, NULL, SW_SHOWNORMAL);
                        } else {
                             ShellExecute(NULL, "open", items[selected].path, NULL, NULL, SW_SHOWNORMAL);
                        }
                    } else {
                        /* Normal Enter Action */
                        if (mode == MODE_FAVORITES || mode == MODE_DRIVES) {
                            strcpy(current_path, items[selected].path);
                            mode = MODE_BROWSE;
                            scan_directory(current_path, &count);
                            selected = 0;
                        } else {
                            /* Browse Mode */
                            if (strcmp(items[selected].label, "..") == 0) {
                                go_up_directory(current_path);
                                scan_directory(current_path, &count);
                                selected = 0;
                            } else if (items[selected].is_dir) {
                                strcpy(current_path, items[selected].path);
                                scan_directory(current_path, &count);
                                selected = 0;
                            } else {
                                ShellExecute(NULL, "open", items[selected].path, NULL, NULL, SW_SHOWNORMAL);
                            }
                        }
                    }
                    clear_screen();
                    break;
                case KEY_BACKSPACE:
                    if (mode == MODE_BROWSE) {
                        /* If length > 3 (e.g. "C:\"), go up. If "C:\", go to drives */
                        if (strlen(current_path) > 3) { 
                            go_up_directory(current_path);
                            scan_directory(current_path, &count);
                            selected = 0;
                        } else {
                            mode = MODE_DRIVES; 
                            scan_drives(&count);
                            selected = 0;
                        }
                        clear_screen();
                    }
                    break;
                case 'd':
                case 'D':
                    if (mode != MODE_DRIVES) {
                        prev_mode = mode;
                        mode = MODE_DRIVES;
                        scan_drives(&count);
                        selected = 0;
                        clear_screen();
                    }
                    break;
                case KEY_ESC:
                    if (mode == MODE_BROWSE) {
                        mode = MODE_FAVORITES;
                        load_config(&count);
                        selected = 0;
                        clear_screen();
                    } else if (mode == MODE_DRIVES) {
                        mode = prev_mode;
                        if (mode == MODE_FAVORITES) {
                            load_config(&count);
                        } else {
                            scan_directory(current_path, &count);
                        }
                        selected = 0;
                        clear_screen();
                    } else {
                        running = 0;
                    }
                    break;
                case 'q':
                case 'Q':
                    running = 0;
                    break;
            }
        }
    }

    clear_screen();
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return 0;
}