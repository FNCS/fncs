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

Json::Value testFederatePublish(const Json::Value recievedValue) {
	Json::Value publishedValue = recievedValue;
	publishedValue["testFederate1"]["intProperty"]["propertyValue"] = publishedValue["testFederate"]["testFederate1"]["intProperty"]["propertyValue"].asInt() + 1;
	publishedValue["testFederate1"]["doubleProperty"]["propertyValue"] = publishedValue["testFederate"]["testFederate1"]["doubleProperty"]["propertyValue"].asDouble() * 3.0;
	publishedValue["testFederate1"]["stringProperty"]["propertyValue"] = publishedValue["testFederate"]["testFederate1"]["stringProperty"]["propertyValue"].asString().append(" blue");
	return publishedValue;
}

int main() {
	Json::Value jsonConfig;
	Json::Value jsonPublish;
	Json::FastWriter jsonWriter;
	Json::StyledStreamWriter jsonStreamWriter;
	Json::Reader jsonReader;
	string jsonConfigString;
	int regIntVal;
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
	jsonConfig["values"];
	jsonConfig["values"]["testKey"];
	jsonConfig["values"]["testKey"]["topic"] = "testFederate_testFederate1/testKey";
	jsonConfig["values"]["testKey"]["default"] = 12;
	jsonConfig["values"]["testKey"]["type"] = "integer";
	jsonConfig["values"]["testKey"]["list"] = "False";
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
		string initialTestKey = fncs::get_value("testKey");
		jsonReader.parse(initialValue, initialValueObject);
		jsonStreamWriter.write(cout, initialValueObject);
		for(int i = 0; i < 10; i++) {
			string agentValue = fncs::agentGetEvents();
			Json::Value agentValueObject;
			if(!agentValue.empty()) {
				jsonReader.parse(agentValue, agentValueObject);
				jsonStreamWriter.write(cout, agentValueObject);
				jsonPublish = testFederatePublish(agentValueObject["testFederate"]);
			}
			//TODO: modify regulat topic subscription for publishing.
			string publishValue = jsonWriter.write(jsonPublish);
			fncs::agentPublish(publishValue);
			fncs::publish("testKey", "13");
			fncs::time_request((fncs::time)(i+1));
		}
		fncs::finalize();
	}
	return 0;
}



