﻿/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <RmlUi/Core.h>
#include <RmlUi/Controls.h>
#include <RmlUi/Debugger.h>
#include <Input.h>
#include <Shell.h>
#include <ShellRenderInterfaceOpenGL.h>


class DemoWindow
{
public:
	DemoWindow(const Rml::Core::String &title, const Rml::Core::Vector2f &position, Rml::Core::Context *context)
	{
		using namespace Rml::Core;
		document = context->LoadDocument("basic/benchmark/data/benchmark.rml");
		if (document != nullptr)
		{
			{
				document->GetElementById("title")->SetInnerRML(title);
				document->SetProperty(PropertyId::Left, Property(position.x, Property::PX));
				document->SetProperty(PropertyId::Top, Property(position.y, Property::PX));
			}

			document->Show();
		}
	}

	void performance_test()
	{
		/*
		  FPS values
		  Original: 18.5  [957f723]
		  Without property counter: 22.0
		  With std::string: 23.0  [603fd40]
		  robin_hood unordered_flat_map: 24.0  [709852f]
		  Avoid dirtying em's: 27.5
		  Restructuring update loop: 34.5  [f9892a9]
		  Element constructor, remove geometry database, remove update() from Context::render: 38.0  [1aab59e]
		  Replace Dictionary with unordered_flat_map: 40.0  [b04b4e5]
		  Dirty flag for structure changes: 43.0  [fdf6f53]
		  Replacing containers: 46.0  [c307140]
		  Replace 'resize' event with virtual function call: 53.0  [7ad658f]
		  Use all_properties_dirty flag when constructing elements: 55.0 [fa6bd0a]
		  Don't double create input elements: 58.0  [e162637]
		  Memory pool for ElementMeta: 59.0  [ece191a]
		  Include chobo flat containers: 65.0  [1696aa5]
		  Move benchmark to its own sample (no code change, fps increase because of removal of animation elements): 68.0  [2433880]
		  Keep the element's main sizing box local: 69.0  [cf928b2]
		  Improved hashing of element definition: 70.0  [5cb9e1d]
		  First usage of computed values (font): 74.0  [04dc275]
		  Computed values, clipping: 77.0
		  Computed values, background-color, image-color, opacity: 77.0
		  Computed values, padding, margin border++: 81.0  [bb70d38]
		  Computing all the values (only using a few of them yet): 83.0  [9fe9bdf]
		  Computed transform and other optimizations: 86.0  [654fa09]
		  Computed layout engine: 90.0   [e18ac30]
		  Replace style cache by computed values: 96.0
		  More computed values: 100.0  [edc78bb] !Woo!
		  Avoid duplicate ToLower++: 103.0  [dec4ef6]
		  Cleanup and smaller changes: 105.0  [38a559d]
		  Move dirty properties to elementstyle: 113.0  [0bba316]
		  (After Windows feature update and MSVC update, no code change): 109.0  [0bba316]
		  Fixes and element style iterators: 108.0  [0bba316]
		  Update definition speedup: 115.0  [5d138fa]
		  (Full release mode, no code change): 135.0  [5d138fa]
		  EventIDs: 139.0  [d2c3956]
		  More on events and EventIDs: 146.0  [fd44d9c]
		  New decorator and font-effect syntax and misc (on a considerably lower clocked CPU): 156 [6619ab4]
		  Removal of manual reference counting: 170 [cb347e1]
		  
		*/

		if (!document)
			return;

		Rml::Core::String rml;

		for (int i = 0; i < 50; i++)
		{
			int index = rand() % 1000;
			int route = rand() % 50;
			int max = (rand() % 40) + 10;
			int value = rand() % max;
			Rml::Core::String rml_row = Rml::Core::CreateString(1000, R"(
			<div class="row">
				<div class="col col1"><button class="expand" index="%d">+</button>&nbsp;<a>Route %d</a></div>
				<div class="col col23"><input type="range" class="assign_range" min="0" max="%d" value="%d"/></div>
				<div class="col col4">Assigned</div>
				<select>
					<option>Red</option><option>Blue</option><option selected>Green</option><option style="background-color: yellow;">Yellow</option>
				</select>
				<div class="inrow unmark_collapse">
					<div class="col col123 assign_text">Assign to route</div>
					<div class="col col4">
						<input type="submit" class="vehicle_depot_assign_confirm" quantity="0">Confirm</input>
					</div>
				</div>
			</div>)",
				index, 
				route,
				max,
				value
			);
			rml += rml_row;
		}

		if (auto el = document->GetElementById("performance"))
			el->SetInnerRML(rml);
	}

	class SimpleEventListener : public Rml::Core::EventListener {
	public:
		void ProcessEvent(Rml::Core::Event& event) override {
			static int i = 0;
			event.GetTargetElement()->SetProperty("background-color", i++ % 2 == 0 ? "green" : "orange");
		}
	} simple_event_listener;

	void click_test()
	{
		if (!document)
			return;

		Rml::Core::String rml;
		
		static int i = 0;
		if(i++ % 2 == 0)
			rml = Rml::Core::CreateString(1000, R"( <div style="width: 100px; height: 100px; background-color: #c33;"/> )");
		else
			rml = "<p>Wohooo!!!</p>";

		if (auto el = document->GetElementById("click_test"))
		{
			el->SetInnerRML(rml);
			if (auto child = el->GetChild(0))
				child->AddEventListener(Rml::Core::EventId::Mouseup, &simple_event_listener);
		}
	}



	~DemoWindow()
	{
		if (document)
		{
			document->Close();
		}
	}

	Rml::Core::ElementDocument * GetDocument() {
		return document;
	}

private:
	Rml::Core::ElementDocument *document;
};


Rml::Core::Context* context = nullptr;
ShellRenderInterfaceExtensions *shell_renderer;
DemoWindow* window = nullptr;

bool run_loop = true;
bool single_loop = true;
bool run_update = true;
bool single_loop_update = true;

void GameLoop()
{
	if (run_update || single_loop_update)
	{
		window->performance_test();
		//window->click_test();

		single_loop_update = false;
	}

	if (run_loop || single_loop)
	{
		context->Update();

		shell_renderer->PrepareRenderBuffer();
		context->Render();
		shell_renderer->PresentRenderBuffer();

		single_loop = false;
	}

	static constexpr int buffer_size = 200;
	static float fps_buffer[buffer_size] = {};
	static int buffer_index = 0;

	static double t_prev = 0.0f;
	double t = Shell::GetElapsedTime();
	float dt = float(t - t_prev);
	t_prev = t;
	static int count_frames = 0;
	count_frames += 1;

	float fps = 1.0f / dt;
	fps_buffer[buffer_index] = fps;
	buffer_index = (++buffer_index % buffer_size);

	if (window && count_frames > buffer_size / 8)
	{
		float fps_mean = 0;
		for (int i = 0; i < buffer_size; i++)
			fps_mean += fps_buffer[(buffer_index + i) % buffer_size];
		fps_mean = fps_mean / (float)buffer_size;

		auto el = window->GetDocument()->GetElementById("fps");
		count_frames = 0;
		el->SetInnerRML(Rml::Core::CreateString( 20, "FPS: %f", fps_mean ));
	}
}



class Event : public Rml::Core::EventListener
{
public:
	Event(const Rml::Core::String& value) : value(value) {}

	void ProcessEvent(Rml::Core::Event& event) override
	{
		using namespace Rml::Core;

		if(value == "exit")
			Shell::RequestExit();

		if (event == "keydown")
		{
			auto key_identifier = (Rml::Core::Input::KeyIdentifier)event.GetParameter< int >("key_identifier", 0);

			if (key_identifier == Rml::Core::Input::KI_SPACE)
			{
				run_loop = !run_loop;
			}
			else if (key_identifier == Rml::Core::Input::KI_DOWN)
			{
				run_loop = false;
				single_loop = true;
			}
			else if (key_identifier == Rml::Core::Input::KI_RIGHT)
			{
				run_update = false;
				single_loop_update = true;
			}
			else if (key_identifier == Rml::Core::Input::KI_RETURN)
			{
				run_update = !run_update;
			}
			else if (key_identifier == Rml::Core::Input::KI_ESCAPE)
			{
				Shell::RequestExit();
			}
			else if (key_identifier == Rml::Core::Input::KI_F8)
			{
				Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
			}
		}
	}

	void OnDetach(Rml::Core::Element* element) override { delete this; }

private:
	Rml::Core::String value;
};


class EventInstancer : public Rml::Core::EventListenerInstancer
{
public:

	/// Instances a new event handle for Invaders.
	Rml::Core::EventListener* InstanceEventListener(const Rml::Core::String& value, Rml::Core::Element* element) override
	{
		return new Event(value);
	}
};


#if defined RMLUI_PLATFORM_WIN32
#include <windows.h>
int APIENTRY WinMain(HINSTANCE RMLUI_UNUSED_PARAMETER(instance_handle), HINSTANCE RMLUI_UNUSED_PARAMETER(previous_instance_handle), char* RMLUI_UNUSED_PARAMETER(command_line), int RMLUI_UNUSED_PARAMETER(command_show))
#else
int main(int RMLUI_UNUSED_PARAMETER(argc), char** RMLUI_UNUSED_PARAMETER(argv))
#endif
{
#ifdef RMLUI_PLATFORM_WIN32
	RMLUI_UNUSED(instance_handle);
	RMLUI_UNUSED(previous_instance_handle);
	RMLUI_UNUSED(command_line);
	RMLUI_UNUSED(command_show);
#else
	RMLUI_UNUSED(argc);
	RMLUI_UNUSED(argv);
#endif

	const int width = 1800;
	const int height = 1000;

	ShellRenderInterfaceOpenGL opengl_renderer;
	shell_renderer = &opengl_renderer;

	// Generic OS initialisation, creates a window and attaches OpenGL.
	if (!Shell::Initialise() ||
		!Shell::OpenWindow("Benchmark Sample", shell_renderer, width, height, true))
	{
		Shell::Shutdown();
		return -1;
	}

	// RmlUi initialisation.
	Rml::Core::SetRenderInterface(&opengl_renderer);
	opengl_renderer.SetViewport(width, height);

	ShellSystemInterface system_interface;
	Rml::Core::SetSystemInterface(&system_interface);

	Rml::Core::Initialise();

	// Create the main RmlUi context and set it on the shell's input layer.
	context = Rml::Core::CreateContext("main", Rml::Core::Vector2i(width, height));
	if (context == nullptr)
	{
		Rml::Core::Shutdown();
		Shell::Shutdown();
		return -1;
	}

	Rml::Controls::Initialise();
	Rml::Debugger::Initialise(context);
	Input::SetContext(context);
	shell_renderer->SetContext(context);

	EventInstancer event_listener_instancer;
	Rml::Core::Factory::RegisterEventListenerInstancer(&event_listener_instancer);

	Shell::LoadFonts("assets/");

	window = new DemoWindow("Benchmark sample", Rml::Core::Vector2f(81, 100), context);
	window->GetDocument()->AddEventListener(Rml::Core::EventId::Keydown, new Event("hello"));
	window->GetDocument()->AddEventListener(Rml::Core::EventId::Keyup, new Event("hello"));
	window->GetDocument()->AddEventListener(Rml::Core::EventId::Animationend, new Event("hello"));


	Shell::EventLoop(GameLoop);

	delete window;

	// Shutdown RmlUi.
	Rml::Core::Shutdown();

	Shell::CloseWindow();
	Shell::Shutdown();

	return 0;
}
