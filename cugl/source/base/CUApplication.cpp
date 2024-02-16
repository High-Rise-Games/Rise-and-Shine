//
//  CUApplication.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides the core application class.  In initializes both the
//  SDL and CUGL settings, and creates the core loop.  You should inherit from
//  this class to make your root game class.
//
//  This class is always intended to be used on the stack of the main function.
//  Thererfore, this class has no allocators.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/1/16
#include <cugl/base/CUApplication.h>
#include <cugl/base/CUDisplay.h>
#include <cugl/render/CUTexture.h>
#include <cugl/input/CUInput.h>
#include <cugl/util/CUDebug.h>
#include <algorithm>
#include <vector>
#include <SDL_atk.h>

/** The default screen width */
#define DEFAULT_WIDTH   1024
/** The default screen height */
#define DEFAULT_HEIGHT  576
/** The default smoothing window for fps calculation */
#define FPS_WINDOW      10
/** The default number of files for streaming */
#define STREAM_LIMIT	8

using namespace cugl;

/** A weak pointer to the single application that is running */
Application* Application::_theapp = nullptr;


#pragma mark -
#pragma mark Constructors
/**
 * Creates a degnerate application with no OpenGL context
 *
 * You must initialize the application to use it.  However, you may
 * set any of the attributes before initialization.
 */
Application::Application() :
_name("CUGL Game"),
_org("GDIAC"),
_savesdir(""),
_assetdir(""),
_state(State::NONE),
_fullscreen(false),
_centered(true),
_highdpi(true),
_fps(0),
_vsync(true),
_funcid(0),
_fixstep(0),
_fixedCounter(0),
_fixedRemainder(0),
_fixed(false),
_clearColor(Color4f::CORNFLOWER) // Ah, XNA
{
    _display.size.set(DEFAULT_WIDTH,DEFAULT_HEIGHT);
#if (CU_PLATFORM == CU_PLATFORM_IPHONE || CU_PLATFORM == CU_PLATFORM_ANDROID)
    _fullscreen = true;
#endif

#if (CU_PLATFORM == CU_PLATFORM_WINDOWS || CU_PLATFORM == CU_PLATFORM_MACOS)
	_multisamp = true;
#else
	_multisamp = false;
#endif
}

/**
 * Disposes all of the resources used by this application.
 *
 * A disposed Node has no OpenGL context, and cannot be used.  However,
 * it can be safely reinitialized.
 */
void Application::dispose() {
    _name = "CUGL Game";
    _state = State::NONE;
    _display.set(0,0,DEFAULT_WIDTH,DEFAULT_HEIGHT);
    _fullscreen = false;
    _highdpi = true;
    _fpswindow.clear();
    _clearColor = Color4f::CORNFLOWER;
    _fixed = false;
    _fixstep = 0;
    _fps = 0;
    _fixedCounter = 0;
    _fixedRemainder = 0;
}

/**
 * Initializes this application, creating an OpenGL context.
 *
 * The initialization will use the current value of all of the attributes,
 * like application name, orientation, and size.  These values should be
 * set before calling init().
 *
 * You should not override this method to initialize user-defined attributes.
 * Use the method onStartup() instead.
 *
 * @return true if initialization was successful.
 */
bool Application::init() {
    _state = State::STARTUP;


    // Initializate the video
    Uint32 flags = 0;
    if (_centered) {
        flags |= Display::INIT_CENTERED;
    }
    if (_fullscreen) {
        flags |= Display::INIT_FULLSCREEN;
    }
    if (_highdpi) {
        flags |= Display::INIT_HIGH_DPI;
    }
    if (_multisamp) {
        flags |= Display::INIT_MULTISAMPLED;
    }
    if (_vsync) {
		flags |= Display::INIT_VSYNC;
    }
    if (!Display::start(_name,_display, flags)) {
        return false;
    }
    if (_fullscreen) {
        _display = Display::get()->getBounds();
        _safearea = Display::get()->getSafeBounds();
    } else {
        _safearea = _display;
    }
    
    SDL_DisplayMode mode;
    SDL_GetDisplayMode(0, 0, &mode);
    if (_fps == 0) {
        setFPS(mode.refresh_rate > 0 ? mode.refresh_rate : 60.0f);
    }
    if (_fixstep == 0) {
        _fixstep = 1000000/_fps;
    }
    
    _fpswindow.resize(FPS_WINDOW,1.0f/_fps);
#ifndef CU_VULKAN
    SDL_GL_SetSwapInterval(_vsync ? 1 : 0);
    Texture::getBlank(); // Prevent this from happening in loading threads
#endif
	ATK_init(STREAM_LIMIT);
    Input::start();
    Application::_theapp = this;
    _boot.mark();
    _state = State::STARTUP;
    
    _fixedCounter = 0;
    _fixedRemainder = 0;
    
    return true;
}

#pragma mark -
#pragma mark Virtual Methods
/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void Application::onStartup() {
    // Switch states and show to user
    Display::get()->show();
    _state = State::FOREGROUND;
    _start.mark();
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void Application::onShutdown() {
    // Switch states
    Input::stop();
    ATK_quit();
    _state = State::NONE;
}

#pragma mark -
#pragma mark Application Loop

/**
 * Gathers SDL input and distributes it to the event handlers.
 *
 * Input is gathered at the state of the animation frame, before update
 * is called.  As it sends all of its information to the appropriate
 * handlers, you should never need to override this method.
 *
 * @return false if the input indicates that the application should quit.
 */
bool Application::getInput() {
    SDL_Event event;
    
    // Process input
    Input::get()->clear();
    while ( SDL_PollEvent(&event) ) {
        if (!Input::get()->update(event)) {
            return false;
        }
        switch (event.type) {
            // APPLICATION STATE
            case SDL_APP_TERMINATING:
                _state = State::SHUTDOWN;
                return false;
                break;
            case SDL_APP_LOWMEMORY:
                onLowMemory();
                break;
            case SDL_APP_WILLENTERBACKGROUND:
                if (_state == State::FOREGROUND) {
                    onSuspend();
                }
                break;
            case SDL_APP_DIDENTERBACKGROUND:
                _state = State::BACKGROUND;
                return false;
                break;
            case SDL_APP_WILLENTERFOREGROUND:
                if (_state == State::BACKGROUND) {
                    onResume();
                }
                break;
            case SDL_APP_DIDENTERFOREGROUND:
                _state = State::FOREGROUND;
                break;
            case SDL_QUIT:
                _state = State::SHUTDOWN;
                return false;
                break;
            default:
                // Ignore the event.
                break;
        }
    }
    
    return true;
}

/**
 * Processes a single animation frame.
 *
 * This method processes the input, calls the update method, and then
 * draws it.  It also updates any running statics, like the average FPS.
 *
 * @return false if the application should quit next frame
 */
bool Application::step() {
    // Get input before doing the next time
    bool running = getInput();

    // Get a (more) precising measurement for simulation
    Timestamp current;
    Uint32 micros   = (Uint32)current.ellapsedMicros(_start);
    _start.mark();
    if (running &&  _state == State::FOREGROUND) {
        processCallbacks((micros)/1000);

        _fpswindow.pop_front();
        _fpswindow.push_back(1000000.0f/micros);
        
        Uint32 simtime = micros + _fixedRemainder;

        if (_fixed) {
            preUpdate(micros / 1000000.0f);

            for (; simtime >= _fixstep; simtime -= _fixstep) {
                fixedUpdate();
                _fixedCounter++;
            }
            _fixedRemainder = simtime;

            postUpdate(micros / 1000000.0f);
        } else {
            update(micros/1000000.0f);
        }
        
		Display::get()->clear(_clearColor);
        draw();
        Display::get()->refresh();
    } else {
        running = _state == State::BACKGROUND;
    }

	// Sleep the remainder
    current.mark();
    Uint32 millis = (Uint32)current.ellapsedMillis(_finish)+1;
    if (millis < _delay) {
		SDL_Delay(_delay - millis);
	}
    
    _finish.mark();
    return running;
}

/**
 * Cleanly shuts down the application.
 *
 * This method will shutdown the application in a way that is guaranteed
 * to call onShutdown() for clean-up.  You should use this method instead
 * of a general C++ exit() function.
 *
 * This method uses the SDL event system.  Therefore, the application will
 * quit at the start of the next animation frame, when all events are
 * processed.
 */
void Application::quit() {
    SDL_Event quit;
    quit.type = SDL_QUIT;
    SDL_PushEvent(&quit);
}

/**
 * Schedules a callback function time milliseconds in the future.
 *
 * This method allows the user to delay an operation until a certain
 * length of time has passed.  If time is 0, it will be called the next
 * animation frame.  Otherwise, it will be called the first animation
 * frame equal to or more than time steps in the future (so there is
 * no guarantee that the callback will be invoked at exactly time
 * milliseconds in the future).
 *
 * At any given time, the callback can terminate by returning false. Once
 * that happens, it will not be invoked again.  Otherwise, the callback
 * will continue to be executed on a regular basis.  After each call, the
 * timer will be reset and it will not be called for another time milliseconds.  
 * If the callback started late, that extra time waited will be credited to 
 * the next call.
 *
 * The callback is guaranteed to be executed in the main thread, so it
 * is safe to access the OpenGL context or any low-level SDL operations.
 * It will be executed after the input has been processed, but before
 * the main {@link update} thread.
 *
 * @param callback  The callback function
 * @param time      The number of milliseconds to delay the callback.
 *
 * @return a unique identifier to unschedule the callback
 */
Uint32 Application::schedule(std::function<bool()> callback, Uint32 time) {
    scheduable item;
    item.callback = callback;
    item.period = time;
    item.timer  = time;
	{
		std::unique_lock<std::mutex> lk(_queueMutex);
		_callbacks.emplace(_funcid++, item);
	}
    return _funcid-1;
}

/**
 * Schedules a reoccuring callback function time milliseconds in the future.
 *
 * This method allows the user to delay an operation until a certain
 * length of time has passed.  If time is 0, it will be called the next
 * animation frame.  Otherwise, it will be called the first animation
 * frame equal to or more than time steps in the future (so there is
 * no guarantee that the callback will be invoked at exactly time
 * milliseconds in the future).
 *
 * At any given time, the callback can terminate by returning false. Once
 * that happens, it will not be invoked again.  Otherwise, the callback
 * will continue to be executed on a regular basis.  After each call, the
 * timer will be reset and it will not be called for another period 
 * milliseconds. If the callback started late, that extra time waited will 
 * be credited to the next call.
 *
 * The callback is guaranteed to be executed in the main thread, so it
 * is safe to access the OpenGL context or any low-level SDL operations.
 * It will be executed after the input has been processed, but before
 * the main {@link update} thread.
 *
 * @param callback  The callback function
 * @param time      The number of milliseconds to delay the callback.
 *
 * @return a unique identifier to unschedule the callback
 */
Uint32 Application::schedule(std::function<bool()> callback, Uint32 time, Uint32 period) {
    scheduable item;
    item.callback = callback;
    item.period = period;
    item.timer  = time;
	{
		std::unique_lock<std::mutex> lk(_queueMutex);
		_callbacks.emplace(_funcid++, item);
	}
    return _funcid-1;
}

/**
 * Stops a callback function from being executed.
 *
 * This method may be used to disable a reoccuring callback. If called
 * soon enough, it can also disable a one-time callback that is yet to
 * be executed.  Once unscheduled, a callback must be re-scheduled in
 * order to be activated again.
 *
 * The callback is identified by its function pointer. Therefore, you
 * should be careful when scheduling anonymous closures.
 */
void Application::unschedule(Uint32 id) {
	std::unique_lock<std::mutex> lk(_queueMutex);
    auto it = _callbacks.find(id);
    if (it != _callbacks.end()) {
        _callbacks.erase(it);
    }
}

/**
 * Processes all of the scheduled callback functions.
 *
 * This method wakes up any sleeping callbacks that should be executed.
 * If they are a one time callback, or if they return false, they are deleted.  
 * If they are a reoccuring callback and return true, the timer is reset.
 *
 * @param millis    The number of milliseconds since last called
 */
void Application::processCallbacks(Uint32 millis) {
	std::vector<Uint32> indeces;
	std::vector<scheduable> actives;
	{
		std::unique_lock<std::mutex> lk(_queueMutex);
		for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it) {
			if (it->second.timer < millis) {
				indeces.push_back(it->first);
				actives.push_back(it->second);
				it->second.timer -= std::min(it->second.timer, millis);
				it->second.timer += it->second.period;
			} else {
				it->second.timer -= millis;
			}
		}
	}

	// These can take a while, so do them outside lock
	for (int ii = 0; ii < actives.size(); ii++) {
		if (actives[ii].callback()) {
			indeces[ii] = (Uint32)-1;
		}
	}

	{
		std::unique_lock<std::mutex> lk(_queueMutex);
		for (auto it = indeces.begin(); it != indeces.end(); ++it) {
			if (*it != (Uint32)-1) {
				_callbacks.erase(*it);
			}
		}
	}
}


#pragma mark -
#pragma mark Initialization Attributes
/**
 * Sets the screen size of this application.
 *
 * If the application is set to be full screen, this value will be ignored.
 * Instead, the application size will be the same as the {@link Display}.
 *
 * This method may only be safely called before the application is
 * initialized.  Once the application is initialized; this value may not
 * be changed.
 *
 * @param width     The screen width
 * @param height    The screen height
 */
void Application::setDisplaySize(int width, int height) {
    CUAssertLog(_state == State::NONE, "Cannot reset application display after initialization");
    _display.size.set((float)width, (float)height);
}

/**
 * Sets the position of the game window
 *
 * If the application is set to be full screen, this value will be ignored.
 * Otherwise, this value specifies the location of the top left corner of
 * the window on the display. By default, a window is centered in the
 * display.
 *
 * This method may only be safely called before the application is
 * initialized.  Once the application is initialized; this value may not
 * be changed.
 *
 * @param x     The x-coordinate of the top-left corner
 * @param y     The y-coordinate of the top-left corner
 */
void Application::setDisplayPosition(int x, int y) {
    CUAssertLog(_state == State::NONE, "Cannot reset application display after initialization");
    _centered = false;
    _display.origin.set(x,y);
}

/**
 * Sets whether this application is running fullscreen
 *
 * Mobile devices must always run fullscreen, and can never be windowed. In
 * addition, this method may only be safely called before the application
 * is initialized.  Once the application is initialized; this value may not
 * be changed.
 *
 * On desktop/laptop platforms, going fullscreen will hide the mouse.
 * The mouse cursor is only visible in windowed mode.
 *
 * @parm value  Whether this application is running fullscreen
 */
void Application::setFullscreen(bool value) {
    CUAssertLog(_state == State::NONE, "Cannot reset application display after initialization");
#if (CU_PLATFORM == CU_PLATFORM_IPHONE || CU_PLATFORM == CU_PLATFORM_ANDROID)
    CUAssertLog(false, "Cannot change fullscreen on mobile devices");
#endif
    _fullscreen = value;
}

/**
 * Sets whether this application supports high dpi resolution.
 *
 * For devices that have high dpi screens (e.g. a pixel ration greater
 * than 1), this will enable that feature.  Otherwise, this value will
 * do nothing.
 *
 * Setting high dpi to true is highly recommended for devides that support
 * it (e.g. iPhones).  It makes the edges of textures much smoother.
 * However, rendering is slightly slower as it effectively doubles (and in
 * some cases triples) the resolution.
 *
 * This method may only be safely called before the application is
 * initialized.  Once the application is initialized; this value may not
 * be changed.
 *
 * @param highDPI   Whether to enable high dpi
 */
void Application::setHighDPI(bool highDPI) {
    CUAssertLog(_state == State::NONE, "Cannot reset application display after initialization");
    _highdpi = highDPI;
}

/**
 * Sets whether this application supports graphics multisampling.
 *
 * Multisampling adds anti-aliasing to OpenGL so that polygon edges are
 * not so hard and jagged.  This does add some extra overhead, and is
 * not really necessary on Retina or high DPI displays.  However, it is
 * pretty much a must in Windows and normal displays.
 *
 * By default, this is false on any platform other than Windows.
 *
 * @param flag	Whether this application should support graphics multisampling.
 */
void Application::setMultiSampled(bool flag) {
	CUAssertLog(_state == State::NONE, "Cannot reset application display after initialization");
#if CU_GL_PLATFORM == CU_GL_OPENGLES
    CUAssertLog(false, "Multisampling is not supported in OpenGLES");
#endif
	_multisamp = flag;
}


#pragma mark -
#pragma mark Runtime Attributes
/**
 * Sets the name of this application
 *
 * On a desktop, the name will be displayed at the top of the window. The
 * name also defines the preferences directory -- the place where it is
 * safe to write save files.
 *
 * This method may be safely changed at any time while the application
 * is running.
 *
 * @param name  The name of this application
 */
void Application::setName(const char* name) {
    _name = name;
    Display* display = Display::get();
    if (display != nullptr) {
        display->setTitle(name);
    }
    _savesdir.clear();
}

/**
 * Sets the name of this application
 *
 * On a desktop, the name will be displayed at the top of the window.
 *
 * This method may be safely changed at any time while the application
 * is running.
 *
 * @param name  The name of this application
 */
void Application::setName(const std::string name) {
    _name = name;
    Display* display = Display::get();
    if (display != nullptr) {
        display->setTitle(name);
    }
    _savesdir.clear();
}

/**
 * Sets the organization name for this application
 *
 * This name defines the preferences directory -- the place where it is
 * safe to write save files. Applications of the same organization will
 * save in the same location.
 *
 * This method may be safely changed at any time while the application
 * is running.
 *
 * @param name  The organization name for this application
 */
void Application::setOrganization(const char* name) {
    _org = name;
    _savesdir.clear();
}

/**
 * Sets the organization name for this application
 *
 * This name defines the preferences directory -- the place where it is
 * safe to write save files. Applications of the same organization will
 * save in the same location.
 *
 * This method may be safely changed at any time while the application
 * is running.
 *
 * @param name  The organization name for this application
 */
void Application::setOrganization(const std::string name) {
    _org = name;
    _savesdir.clear();
}

/**
 * Sets whether this application obeys the display refresh rate
 *
 * A vsync-enabled application will always match the refresh rate of
 * the display. Otherwise, the application will attempt to match the
 * value of {@link #getFPS()}, which could be faster than the refresh
 * rate.
 *
 * Note that some platforms (notably macOS) will always use vsync
 * no matter the settings. In that case, setting this value to false
 * will actually hurt the performance of your application. As a
 * general rule, it is best to set this value to true, and perform
 * any simulations that must be done at a faster rate in a separate
 * thread.
 *
 * @param vsync Whether this application obeys the display refresh rate
 */
void Application::setVSync(bool vsync) {
    _vsync = vsync;
#ifdef CU_VULKAN
    // TODO: Add runtime Vsync to Vulkan
#else
    if (_state != State::NONE) {
        SDL_GL_SetSwapInterval(_vsync ? 1 : 0);
    }
#endif
}

/**
 * Sets the target frames per second of this application.
 *
 * The application does not guarantee that the fps target will always be
 * met.  In particular, if the update() and draw() methods are expensive,
 * it may run slower. However, it does guarantee that the program never
 * runs faster than this FPS value.
 *
 * This method may be safely changed at any time while the application
 * is running.
 *
 * By default, this value is 60.
 *
 * @param fps   The target frames per second
 */
void Application::setFPS(float fps) {
    _fps = fps;
    if (_fps > 0 && _fps <= 500.0f) {
        _delay = (int)(1000.0f/_fps);
        _delay -= 1;        // To keep us from getting ahead of VSync
    } else {
        _delay = 0;
    }
}

/**
 * Returns the average frames per second over the last 10 frames.
 *
 * The method provides a way of computing the curren frames per second that
 * smooths out any one-frame anomolies.  The FPS is averages over the
 * exact rate of the past 10 frames.
 *
 * @return the average frames per second over the last 10 frames.
 */
float Application::getAverageFPS() const {
    float total = 0;
    for(auto it=_fpswindow.begin(); it != _fpswindow.end(); ++it) {
        total += *it;
    }
    return total/_fpswindow.size();
}

/**
 * Sets the simulation timestep of this application.
 *
 * The value defines the rate at which {@link #fixedUpdate} is called.
 * The rate is a logical value, not a wall-clock value.  That is, if
 * {@link #draw} is called at time T, then the method {@link #fixedUpdate}
 * will have been called T/s times, where s is the simulation timestep.
 *
 * This timestep is set in microseconds for the purposes of precision.
 * Note that this value does nothing if {@link #setDeterministic} is set
 * to false. In that case, {@link #fixedUpdate} is never called, and
 * {@link #update} is called instead.
 *
 * This method may be safely changed at any time while the application
 * is running. By default, this value will match the initial FPS of the
 * application.
 *
 * @param step  The simulation timestep
 */
void Application::setFixedStep(Uint64 step) {
    _fixstep = step;
}

/**
 * Instructs the application to use the deterministic loop.
 *
 * If this value is set to false, then the application will use the simple
 * structure of alternating between {@link #update} and {@link #draw}.
 * However, if it is set to true, it will use a more complicated loop in
 * place of {@link #update}, consisting of a call to {@link #preUpdate},
 * followed by zero or more calls to {@link #fixedUpdate}.
 *
 * This method may be safely changed at any time while the application
 * is running. By default, this value is false.
 *
 * @param value Whether to use the deterministic loop
 */
void Application::setDeterministic(bool value) {
    if (value == _fixed) {
        return;
    } else if (value) {
        _fixedCounter = 0;
        _fixedRemainder = 0;
    }
    _fixed = value;
}

/**
 * Returns the OpenGL description for this application
 *
 * @return the OpenGL description for this application
 */
const std::string Application::getOpenGLDescription() const {
    const char* glinfo = (const char*)glGetString(GL_VERSION);
    return std::string(glinfo);
}


#pragma mark -
#pragma mark File Directories
/**
 * Returns the base directory for all assets (e.g. the assets folder).
 *
 * The assets folder is a READ-ONLY folder for providing assets for the
 * game.  Its path depends on the platform involved.  Android uses
 * this to refer to the dedicated assets folder, while MacOS/iOS refers
 * to the resource bundle.  On Windows, this is the working directory.
 *
 * The value returned is an absolute path in UTF-8 encoding, and has the
 * appropriate path separator for the given platform ('\\' on Windows,
 * '/' most other places). In addition, it is guaranteed to end with a
 * path separator, so that you can append a file name to the path.
 *
 * It is possible that the the string is empty.  For example, the assets
 * directory for Android is not a proper directory (unlike the save
 * directory) and should not be treated as such.  In particular, this
 * string should never be converted to a {@link Pathname} object.
 *
 * Asset loaders use this directory by default.
 *
 * @return the base directory for all assets (e.g. the assets folder).
 */
std::string Application::getAssetDirectory() {
#if defined (__ANDROID__)
#elif defined (__WINDOWS__)
	if (_assetdir.empty()) {
		char currDir[255];
		GetCurrentDirectoryA(255, currDir);
		_assetdir = currDir;
		_assetdir.append("\\");
	}
#else
    if (_assetdir.empty()) {
		_assetdir.append(SDL_GetBasePath());
    }
#endif
    return _assetdir;
}

/**
 * Returns the base directory for writing save files and preferences.
 *
 * The save folder is a READ-WRITE folder for storing saved games and
 * preferences.  The folder is unique to the current user.  On desktop
 * platforms, it is typically in the user's home directory.  You must
 * use this folder (and not the asset folder) if you are writing any
 * files.
 *
 * The value returned is an absolute path in UTF-8 encoding, and has the
 * appropriate path separator for the given platform ('\\' on Windows,
 * '/' most other places). In addition, it is guaranteed to end with a
 * path separator, so that you can append a file name to the path.
 *
 * I/O classes (both readers and writers) use this directory by default.
 * However, if you are want to use this directory in an asset loader (e.g.
 * for a saved game file), you you may want to refer to the path directly.
 *
 * @return the base directory for writing save files and preferences.
 */
std::string Application::getSaveDirectory() {
    if (_savesdir.empty()) {
		_savesdir.append(SDL_GetPrefPath(_org.c_str(),_name.c_str()));
    }
    return _savesdir;
}
