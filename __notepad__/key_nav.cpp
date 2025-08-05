#include <iostream>
#include <cstdio>
#include <ncurses.h>
#include <string.h>

int read_file(const char* file) {
    FILE* fp = fopen(file, "r");
    if (!fp) return 0;

    char ch[1024];
    while (fgets(ch, sizeof(ch), fp) != NULL) {
        printw("%s", ch);
    }
    fclose(fp);
    refresh();
    return 1;
}

int main() {
    std::string fln = "notepad_data.txt";
    int cha;
    int x = 0, y = 0;

    initscr();              // Start ncurses
    raw();                  // Disable signal processing (get raw characters)
    keypad(stdscr, TRUE);   // Enable special keys
    noecho();               // Don't echo typed chars
    FILE* fn = fopen(fln.c_str(), "a+");
    read_file(fln.c_str());
    char buffer[1024];
    getyx(stdscr, y, x);
    move(y + 1, 0);         // Move cursor to next line after file print
    printw("Initial Cursor Position: (%d,%d)", x, y);
    refresh();
	
    while (1) {
        cha = getch();
        if (cha == 3 || cha == 26) {  // Ctrl+C or Ctrl+Z
            break;
        } else {
            getyx(stdscr, y, x);
            switch (cha) {
                case KEY_UP:
                    move(y - 1, x);
                    break;
                case KEY_DOWN:
                    move(y + 1, x);
                    break;
                case KEY_LEFT:
                    move(y, x - 1);
                    break;
                case KEY_RIGHT:
                    move(y, x + 1);
                    break;
				case KEY_BACKSPACE:
					getyx(stdscr,y,x);
                    if (x!=0){
                        move(y,x-1);
                        delch();
                    }
                    else{
                        char a[1024];
                        char strin[50];
                        mvinnstr(y, 0, a, sizeof(a) - 1);
                        insdelln(-1);
                        mvinnstr(y-1, 0, strin, sizeof(strin) - 1);
                        mvaddstr(y-1, strlen(strin), a);
                    } 
					break;
				case 10: // Enter key (ASCII '\n') — KEY_ENTER often doesn't trigger as expected

                    getyx(stdscr, y, x);

                    // Read the current line into buffer
                    char line[1024];
                    mvinnstr(y, 0, line, sizeof(line) - 1);

                    // Split into left and right parts at cursor
                    char left[1024], right[1024];
                    strncpy(left, line, x);
                    left[x] = '\0';
                    strcpy(right, line + x);

                    // Overwrite current line with left
                    move(y, 0);
                    clrtoeol();
                    printw("%s", left);

                    // Shift down lines if needed (optional for advanced editor)

                    // Move to next line and print right side
                    move(y + 1, 0);
                    insdelln(1);  // Insert a blank line below
                    printw("%s", right);

                    // Move cursor to start of new line
                    move(y + 1, 0);
                    break;


                default:
                    // Insert typed character at current cursor position
                    getyx(stdscr, y, x);
                    char line[1024];
                    mvinnstr(y, 0, line, sizeof(line) - 1);

                    // Split line at cursor
                    char left[1024], right[1024];
                    strncpy(left, line, x);
                    left[x] = '\0';
                    strcpy(right, line + x);    

                    // Build new line with inserted character
                    char new_line[1024];
                    snprintf(new_line, sizeof(new_line), "%s%c%s", left, cha, right);

                    // Overwrite current line
                    move(y, 0);
                    clrtoeol();
                    printw("%s", new_line);

                    // Move cursor after inserted character
                    move(y, x + 1);
                    break;

            }
            refresh();
        }
    }

    endwin();  // Exit ncurses mode
    return 0;
}