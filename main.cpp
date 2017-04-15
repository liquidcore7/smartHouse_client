#include <iostream>
#include <nana/gui.hpp>
#include <regex>
#include "web/client_http.hpp"
#include <nana/gui/widgets/widget.hpp>
#include <nana/gui/wvl.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/msgbox.hpp>
#include <nana/gui/widgets/button.hpp>
#include <thread>

typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;
bool canBeClosed = true;

int main() {
	nana::color red;
	red.from_rgb(255, 95, 95);

	std::unique_ptr<HttpClient> client;

	nana::form form2(nana::API::make_center(500, 400));
	form2.caption("Controls");

	nana::button light1(form2, nana::rectangle(50, 80, 130, 70));
	nana::button light2(form2, nana::rectangle(50, 170, 130, 70));
	nana::button door(form2, nana::rectangle(220, 80, 130, 70));
	nana::button ext(form2, nana::rectangle(380, 310, 80, 50));

	ext.caption("Quit");
	ext.events().click([&client] ()
	                   {
		                   client->request("POST", "/unauth");
		                   nana::API::exit_all();
	                   });

	light1.caption("Toggle light1");
	light2.caption("Toggle light2");
	door.caption("Open/Close door");
	light1.enable_focus_color(false);
	light2.enable_focus_color(false);
	door.enable_focus_color(false);

	light1.events().click([&light1, &client, &red] ()
	                     {
		                     if (light1.bgcolor() == red) {
			                     client->request("POST", "/setlight1", "ON");
			                     light1.bgcolor(nana::colors::light_green);
		                     }
		                     else {
			                     client->request("POST", "/setlight1", "OFF");
			                     light1.bgcolor(red);
		                     }
	                     });
	light2.events().click([&light2, &client, &red] ()
	                      {
		                      if (light2.bgcolor() == red) {
			                      client->request("POST", "/setlight2", "ON");
			                      light2.bgcolor(nana::colors::light_green);
		                      }
		                      else {
			                      client->request("POST", "/setlight2", "OFF");
			                      light2.bgcolor(red);
		                      }
	                      });
	door.events().click([&door, &client, &red] ()
	                    {
		                    if (door.bgcolor() == red) {
			                    client->request("POST", "/setdoor", "CLOSE");
			                    door.bgcolor(nana::colors::light_green);
		                    }
		                    else {
			                    client->request("POST", "/setdoor", "OPEN");
			                    door.bgcolor(red);
		                    }
	                    });

	nana::form mainForm(nana::API::make_center(400, 150));
	mainForm.caption("Connect to server");
	mainForm.events().destroy([&mainForm, &form2, &canBeClosed] () {
		if (canBeClosed) {mainForm.close(); _Exit(EXIT_SUCCESS);}
	});

	nana::textbox url(mainForm, nana::rectangle(50, 20, 300, 25));
	nana::textbox passw(mainForm, nana::rectangle(50, 55, 300, 25));
	passw.multi_lines(false);
	url.multi_lines(false);
	url.caption("Enter URL");
	passw.caption("Enter password");

	nana::button conn(mainForm, nana::rectangle(150, 100, 120, 45));
	conn.caption("Connect");
	conn.bgcolor(nana::colors::light_green);
	conn.events().click([&url, &passw, &mainForm, &form2, &client, &light1, &light2, &door, &red] () {
		std::string URstr, URpattern("^\\d{1,3}\.\\d{1,3}\.\\d{1,3}\.\\d{1,3}\:\\d{2,5}$");
		url.getline(0, URstr);
		bool urlpass = std::regex_match(URstr, std::regex(URpattern));
		nana::msgbox newFrm;
		if (!urlpass)
		{
			newFrm << "Incorrect URL";
			newFrm.show();
		} else
		{
			try {
				client = std::unique_ptr<HttpClient>(new HttpClient(URstr));
				auto connTest = client->request("GET", "/try");
				std::stringstream tmpss;
				tmpss << connTest->content.rdbuf();
				bool connected = (tmpss.str().find("Connected") != std::string::npos);
				tmpss = std::stringstream();
				std::string pwd;
				passw.getline(0, pwd);
				auto authSucc = client->request("POST", "/auth", pwd);
				tmpss << authSucc->content.rdbuf();
				std::string authState;
				tmpss >> authState;
				tmpss = std::stringstream();
				if (!connected)
				{
					newFrm << "Connection issue. Check your URL and server state";
					newFrm.show();
				} else if (authState.find('1') == std::string::npos)
				{
					newFrm << "Wrong password";
					newFrm.show();
				} else
				{
					newFrm << "Successfully connected to " << URstr;
					newFrm.show();
					canBeClosed = false;
					mainForm.close();
					auto lightState = client->request("GET", "/getstate");
					tmpss << lightState->content.rdbuf();
					char state1, state2, state3;
					tmpss >> state1 >> state2 >> state3;
					light1.bgcolor((state1 == '1') ? nana::colors::light_green : red);
					light2.bgcolor((state2 == '1') ? nana::colors::light_green : red);
					door.bgcolor((state3 == '1') ? nana::colors::light_green : red);
					form2.show();
				}
			} catch(boost::system::system_error &e)
			{
				newFrm << e.what() << std::endl;
				newFrm.show();
			}
		} // if (urlpass)
	});

	url.events().key_char([&conn] (const nana::arg_keyboard &kb) { if (kb.key == 13) nana::click(conn);});

	mainForm.show();
	nana::exec();
	return 0;
}