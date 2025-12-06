#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

#define MAX_APPS 20
#define GRID_ROWS 4
#define GRID_COLS 5

typedef struct {
    char name[256];
    char iconPath[512];
    char exec[512];
    Texture2D texture;
    bool textureLoaded;
} AppEntry;

// Helper to trim whitespace
void Trim(char *str) {
    char *end;
    while(*str == ' ') str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && *end == ' ' || *end == '\n' || *end == '\r') end--;
    *(end+1) = 0;
}

// Simple heuristic to find icon if it's not a full path
void ResolveIconPath(char *iconPath) {
    if (iconPath[0] == '/') return; // Already a path

    // Check common locations (very basic)
    const char *searchPaths[] = {
        "/usr/share/pixmaps/",
        "/usr/share/icons/hicolor/48x48/apps/",
        "/usr/share/icons/hicolor/scalable/apps/",
        "/usr/share/icons/Papirus-Apps/48x48/apps/", 
        NULL
    };

    const char *extensions[] = { ".png", ".svg", ".xpm", NULL };

    char tempPath[512];
    for (int i = 0; searchPaths[i] != NULL; i++) {
        for (int j = 0; extensions[j] != NULL; j++) {
            snprintf(tempPath, sizeof(tempPath), "%s%s%s", searchPaths[i], iconPath, extensions[j]);
            if (FileExists(tempPath)) {
                strcpy(iconPath, tempPath);
                return;
            }
        }
        // Try without extension if it might be in the name or folder structure
        snprintf(tempPath, sizeof(tempPath), "%s%s.png", searchPaths[i], iconPath);
        if (FileExists(tempPath)) {
            strcpy(iconPath, tempPath);
            return;
        }
    }
}

void ParseDesktopFile(const char *path, AppEntry *app) {
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char line[1024];
    bool inEntry = false;
    
    app->name[0] = '\0';
    app->iconPath[0] = '\0';
    app->exec[0] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "[Desktop Entry]", 15) == 0) {
            inEntry = true;
            continue;
        }
        
        // Some files have multiple sections, we mainly care about the first [Desktop Entry]
        // But for simplicity, we just look for keys. 
        // A robust parser would check sections.
        
        if (strncmp(line, "Name=", 5) == 0) {
            char *val = line + 5;
            Trim(val);
            if (app->name[0] == '\0') strncpy(app->name, val, sizeof(app->name) - 1);
        } else if (strncmp(line, "Icon=", 5) == 0) {
            char *val = line + 5;
            Trim(val);
            if (app->iconPath[0] == '\0') strncpy(app->iconPath, val, sizeof(app->iconPath) - 1);
        } else if (strncmp(line, "Exec=", 5) == 0) {
            char *val = line + 5;
            Trim(val);
            if (app->exec[0] == '\0') strncpy(app->exec, val, sizeof(app->exec) - 1);
        } else if (strncmp(line, "NoDisplay=true", 14) == 0) {
             // Skip NoDisplay apps if we wanted to be strict, but for now we load them
             // or we could mark the app as invalid.
        }
    }
    fclose(fp);

    if (app->iconPath[0] != '\0') {
        ResolveIconPath(app->iconPath);
    }
}

void LoadApps(AppEntry *apps, int *count, int maxApps) {
    const char *dirs[] = {
        "/home/guest/.local/share/applications/",
        "/usr/share/applications/",
        NULL
    };

    *count = 0;
    for (int d = 0; dirs[d] != NULL; d++) {
        DIR *dir = opendir(dirs[d]);
        if (!dir) continue;

        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (*count >= maxApps) break;
            
            if (strstr(ent->d_name, ".desktop")) {
                char fullPath[1024];
                snprintf(fullPath, sizeof(fullPath), "%s%s", dirs[d], ent->d_name);
                
                AppEntry newApp;
                ParseDesktopFile(fullPath, &newApp);
                
                if (strlen(newApp.name) > 0) {
                    apps[*count] = newApp;
                    (*count)++;
                }
            }
        }
        closedir(dir);
        if (*count >= maxApps) break;
    }
}

int main() {
    // Enable window transparency
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);
    
    InitWindow(0, 0, "Transparent Launcher");
    
    // Set window to fullscreen size but not exclusive fullscreen mode to keep transparency working well on some WMs
    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);
    SetWindowSize(screenWidth, screenHeight);
    SetWindowPosition(0, 0);

    AppEntry apps[MAX_APPS];
    int appCount = 0;
    LoadApps(apps, &appCount, MAX_APPS);

    // Load Textures
    for (int i = 0; i < appCount; i++) {
        apps[i].textureLoaded = false;
        if (strlen(apps[i].iconPath) > 0 && FileExists(apps[i].iconPath)) {
            Image img = LoadImage(apps[i].iconPath);
            if (img.data != NULL) {
                // Resize for uniformity
                ImageResize(&img, 64, 64); 
                apps[i].texture = LoadTextureFromImage(img);
                apps[i].textureLoaded = true;
                UnloadImage(img);
            }
        }
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        // Clear with semi-transparent black (R,G,B,A)
        // 0.3 alpha is approx 76/255
        ClearBackground((Color){0, 0, 0, 76});

        int cellWidth = screenWidth / GRID_COLS;
        int cellHeight = screenHeight / GRID_ROWS;

        for (int i = 0; i < appCount; i++) {
            int row = i / GRID_COLS;
            int col = i % GRID_COLS;
            
            int x = col * cellWidth;
            int y = row * cellHeight;

            // Draw Icon
            int iconX = x + (cellWidth - 64) / 2;
            int iconY = y + (cellHeight - 64) / 2 - 20; // Shift up a bit for text

            if (apps[i].textureLoaded) {
                DrawTexture(apps[i].texture, iconX, iconY, WHITE);
            } else {
                DrawRectangle(iconX, iconY, 64, 64, GRAY); // Placeholder
            }

            // Draw Name
            int textWidth = MeasureText(apps[i].name, 20);
            DrawText(apps[i].name, x + (cellWidth - textWidth) / 2, iconY + 70, 20, WHITE);
        }

        EndDrawing();
    }

    // Unload Textures
    for (int i = 0; i < appCount; i++) {
        if (apps[i].textureLoaded) {
            UnloadTexture(apps[i].texture);
        }
    }

    CloseWindow();

    return 0;
}
