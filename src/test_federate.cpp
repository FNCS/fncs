/*
 * test_federate.cpp
 *
 *  Created on: Feb 16, 2017
 *      Author: afisher
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include "json-cpp/json/json.h"
#include "fncs.hpp"
using namespace std;

int main() {
	Json::Value jsonConfig;
	Json::Value jsonPublish;
	Json::FastWriter jsonWriter;
	Json::StyledStreamWriter jsonStreamWriter;
	Json::Reader jsonReader;
	string jsonConfigString;
	jsonConfig["agentType"] = "testFederate";
	jsonConfig["agentName"] = "testFederate1";
	jsonConfig["timeDelta"] = "1s";
	jsonConfig["broker"] = "tcp://localhost:5570";
	jsonConfig["publications"];
	jsonConfig["publications"]["intProperty"];
	jsonConfig["publications"]["intProperty"]["propertyType"] = "integer";
	jsonConfig["publications"]["intProperty"]["propertyUnit"] = "";
	jsonConfig["publications"]["intProperty"]["propertyValue"] = 1;
	jsonConfig["publications"]["doubleProperty"];
	jsonConfig["publications"]["doubleProperty"]["propertyType"] = "double";
	jsonConfig["publications"]["doubleProperty"]["propertyUnit"] = "$";
	jsonConfig["publications"]["doubleProperty"]["propertyValue"] = 10.50;
	jsonConfig["publications"]["stringProperty"];
	jsonConfig["publications"]["stringProperty"]["propertyType"] = "string";
	jsonConfig["publications"]["stringProperty"]["propertyUnit"] = "";
	jsonConfig["publications"]["stringProperty"]["propertyValue"] = "red";
	jsonConfig["subscriptions"];
	jsonConfig["subscriptions"]["testFederate"];
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"];
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["intProperty"];
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["intProperty"]["propertyType"] = "integer";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["intProperty"]["propertyUnit"] = "";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["intProperty"]["propertyValue"] = 2;
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["doubleProperty"];
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["doubleProperty"]["propertyType"] = "double";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["doubleProperty"]["propertyUnit"] = "$";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["doubleProperty"]["propertyValue"] = 7.12;
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["stringProperty"];
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["stringProperty"]["propertyType"] = "integer";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["stringProperty"]["propertyUnit"] = "";
	jsonConfig["subscriptions"]["testFederate"]["testFederate1"]["stringProperty"]["propertyValue"] = "green";
	jsonConfigString = jsonWriter.write(jsonConfig);
	fncs::agentRegister(jsonConfigString);
	jsonPublish["testFederate"];
	jsonPublish["testFederate"]["testFederate1"];
	jsonPublish["testFederate"]["testFederate1"]["intProperty"];
	jsonPublish["testFederate"]["testFederate1"]["intProperty"]["propertyType"] = "integer";
	jsonPublish["testFederate"]["testFederate1"]["intProperty"]["propertyUnit"] = "";
	jsonPublish["testFederate"]["testFederate1"]["intProperty"]["propertyValue"] = 2;
	jsonPublish["testFederate"]["testFederate1"]["doubleProperty"];
	jsonPublish["testFederate"]["testFederate1"]["doubleProperty"]["propertyType"] = "double";
	jsonPublish["testFederate"]["testFederate1"]["doubleProperty"]["propertyUnit"] = "$";
	jsonPublish["testFederate"]["testFederate1"]["doubleProperty"]["propertyValue"] = 7.12;
	jsonPublish["testFederate"]["testFederate1"]["stringProperty"];
	jsonPublish["testFederate"]["testFederate1"]["stringProperty"]["propertyType"] = "integer";
	jsonPublish["testFederate"]["testFederate1"]["stringProperty"]["propertyUnit"] = "";
	jsonPublish["testFederate"]["testFederate1"]["stringProperty"]["propertyValue"] = "green";
	if(!fncs::is_initialized()) {
		return EXIT_FAILURE;
	} else {
		Json::Value initialValueObject;
		string initialValue = fncs::get_value("testFederate_testFederate1");
		jsonReader.parse(initialValue, initialValueObject);
		jsonStreamWriter.write(cout, initialValueObject);
		for(int i = 0; i < 10; i++) {
			string agentValue = fncs::agentGetEvents();
			Json::Value agentValueObject;
			if(!agentValue.empty()) {
				jsonReader.parse(agentValue, agentValueObject);
				jsonStreamWriter.write(cout, agentValueObject);
			}
			string publishValue = jsonWriter.write(jsonPublish);
			fncs::agentPublish(publishValue);
			fncs::time_request((fncs::time)(i+1));
		}
		fncs::finalize();
	}
	return 0;
}



