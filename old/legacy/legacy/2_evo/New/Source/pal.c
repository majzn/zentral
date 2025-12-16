/********************************************/
/*   oooo               O        O          */
/*  O                   O     O  O          */
/*  O                   O        O          */
/*   oooo    oo   O  O  O     O  Oooo       */
/*       O  O  O  O O   O     O  O   O      */
/*       O  O  O  Oo    O     O  O   O      */
/*   oooo    oo   O     Oooo  O  Oooo  1.10 */
/*                O                         */
/*                                          */
/*  Graphics, Audio and Input library for   */
/*  Interactive Applications                */
/*                                          */
/********************************************/

/*******************************************/
/*       Copyright (c) 2021 CTech          */
/*         All rights reserved.            */
/*******************************************/
#define PAL_LIB_IMPLEMENTATION
#include "pal.h"

palModule tracker;
palModule evolution;
palApp app;

sflag PAL_render(palHandle hApp, int32 width, int32 height) {
  palAppHandle app = (palAppHandle)hApp;
  palFont prevFont = app->draw.font;
  PAL_drawSetPaletteColour(app, PAL_GUI_NEUTRAL);
  PAL_moduleRender(app, &evolution);
  PAL_fontSelect(app, PAL_FONT_DEFAULT);
  if(!PAL_windowIsFullscreen(app)) {
    PAL_drawBaseGUI(app, width, height);
  }
  app->draw.font = prevFont;
  return TRUE;
}

sflag PAL_resize(palHandle hApp, int32 width, int32 height) {
  palAppHandle app = (palAppHandle)hApp;
  palFont prevFont = app->draw.font;
  PAL_drawSetPaletteColour(app, PAL_GUI_NEUTRAL);
  PAL_moduleResize(app, &evolution);
  PAL_moduleRender(app, &evolution);
  PAL_fontSelect(app, PAL_FONT_DEFAULT);
  if(app->hardwareAccelaration) {
    SGL_viewport(0, 0,
        PAL_width(app)*app->pixelResolution,
        PAL_height(app)*app->pixelResolution);
  }
  if(!PAL_windowIsFullscreen(app)) {
    PAL_drawBaseGUI(app, width, height);
  }
  app->draw.font = prevFont;
  return TRUE;
}

palGLShader defShader;
palGLTexture softTexture;
PAL_VAO triVAO;
PAL_VBO triVBO;

int32 main(int argc, char* argv[]) {
  PAL_appCreate(&app, 640, 480, 1, "maj zalaznik novak 2024", FALSE);
  PAL_loadGUIPalette(&app, "assets/palettes/win2k.pal");
  PAL_windowSetResizeFunc(&app, PAL_resize);
  PAL_appSetTimestep(&app, 60);
  PAL_appSetRenderStep(&app, 1);
  PAL_appSetUpdateStep(&app, 1);
  PAL_initGUI(&app, app.frameBuffer.width, app.frameBuffer.height);
  PAL_printMemoryState(&app);
  PAL_enableSpriteGUI(&app, TRUE);
  /*
  PAL_moduleLoad(&tracker, "paltracker.dll", "tracker", &app);
  */
  PAL_moduleLoad(&evolution, "evolution.dll", "evolution", &app);
  /*
  SGL_shaderCreate(&defShader,
      "assets/shaders/basic_vertex.ss",
      "assets/shaders/basic_fragment.ss");

  SGL_genVertexArrays(1, &triVAO);
  SGL_genBuffers(1, &triVBO);

  SGL_bindVertexArray(triVAO);
  SGL_bindBuffer(GL_ARRAY_BUFFER, triVBO);
  SGL_bufferData(GL_ARRAY_BUFFER, 30*sizeof(GL_FLOAT), quadVerts);

  SGL_vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GL_FLOAT), (GLvoid*)0);
  SGL_vertexAttribEnable(0);
  SGL_vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GL_FLOAT), (GLvoid*)(3*sizeof(GL_FLOAT)));
  SGL_vertexAttribEnable(1);

  SGL_bindBuffer(GL_ARRAY_BUFFER, 0);
  SGL_bindVertexArray(0);

  SGL_genTextures(1, &softTexture);
  SGL_bindTexture(GL_TEXTURE_2D, &softTexture);
  SGL_texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE,
      app.frameBuffer.mem);
  */
  SLOG("Loaded module\n");
  while(!PAL_appShouldQuit(&app)) {
    PAL_appPollInput(&app);
    PAL_appUpdateTiming(&app);
    PAL_appBeginFrame(&app);
    /*
    PAL_moduleReload(&app, &tracker);
    */
    PAL_moduleReload(&app, &evolution);
    /*
    PAL_moduleUpdate(&app, &tracker);
    */
    PAL_moduleUpdate(&app, &evolution);

    PAL_render((palHandle)&app, app.frameBuffer.width, app.frameBuffer.height);
    /*
    SGL_clearColour(1.0f, 1.0f, 1.0f, 1.0f);
    GLCHECK();
    SGL_clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLCHECK();
    SGL_shaderUse(&defShader);
    SGL_shaderSetFloat(&defShader, "sTime", ((real32)appFrames)*0.01f);
    GLCHECK();
    SGL_activeTexture(GL_TEXTURE0);
    SGL_bindTexture(GL_TEXTURE_2D, &softTexture);
    SGL_texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, app.frameBuffer.width,
        app.frameBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
      app.frameBuffer.mem);
    SGL_genMipmaps(GL_TEXTURE_2D);
    SGL_shaderSetInt(&defShader, "softTexture", 0);

    SGL_bindVertexArray(triVAO);
    GLCHECK();
    SGL_drawArrays(GL_TRIANGLES, 0, 6);
    GLCHECK();
    SGL_bindVertexArray(0);
    */
    PAL_appEndFrame(&app);
  }

  PAL_moduleFree(&app, &tracker);
  PAL_moduleFree(&app, &evolution);
  PAL_appDestroy(&app);
  return 0;
}
