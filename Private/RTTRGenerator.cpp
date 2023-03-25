#include "RTTRGenerator.h"

#include "Utility.h"

// RTTRGenerator Utilities
string GetSetFunction(string& funcName)
{
	string name = funcName;

	// lowercase string
	transform(name.begin(), name.end(), name.begin(), LOWERCASE);

	if(name.substr(0, 3) != "get")
		return "";

	name = funcName;

	if(funcName[0] == 'G')
	{
		name[0] = 'S';
	}
	else
	{
		name[0] = 's';
	}

	return name;
}

string ValidatePolicy(ReflectionDefinition& item)
{
	string value = item.ReturnPolicy;

	transform(value.begin(), value.end(), value.begin(), LOWERCASE);

	if (item.Type == PROPERTY) 
	{
		if (value == "asref" || value == "reflectionpolicy::asref") {
			return "as_reference_wrapper";
		}
		else if (value == "aspointer" || value == "reflectionpolicy::aspointer") {
			return "bind_as_ptr";
		}
	}
	else if(item.Type == FUNCTION) 
	{
		if (value == "discard" || value == "reflectionpolicy::discard") {
			return "discard_return";
		}
		else if (value == "aspointer" || value == "reflectionpolicy::aspointer") {
			return "return_ref_as_ptr";
		}
	}

	return "";
}

RTTRGenerator::RTTRGenerator(CmdParser& cmd, ClangParser& parser) : 
	Definitions(parser.Definitions),
	UseClassPath(false),
	EnableHotswap(false)
{
    strm.open(cmd.OutputFile, ios_base::out);

    if(!strm.is_open())
    {
        cerr << "Failed To Open " << cmd.OutputFile.c_str() << " For Writing!" << endl;
    }

    EnableHotswap = cmd.EnableHotswap;
    UseClassPath = cmd.UseClassPath;

    Module = cmd.Module;

    OutputFile = cmd.OutputFile;
}

void RTTRGenerator::Generate()
{
    if (!strm.is_open())
    return;

	// The module class path (/Core/Vector/)
	string Classpath;
	string HeaderNamespace;

	if (UseClassPath)
	{
		if (!Module.empty())
		{
			Classpath += ("/" + Module + "/");
			std::replace_if(Classpath.begin(), Classpath.end(), IsInvalidPathChar, '/');
			ReplaceDuplicates(Classpath);
			HeaderNamespace = Classpath;
			std::replace_if(HeaderNamespace.begin(), HeaderNamespace.end(), IsInvalidNSChar, '_');

			transform(HeaderNamespace.begin(), HeaderNamespace.end(), HeaderNamespace.begin(), ::toupper);
		}
	}

	// The include file to make all this work
	strm << "#include <rttr/registration.h>" << endl << endl;

	// get the header name from file name
	string headerName = OutputFile.filename().generic_string();
	
	// uppercase the header name
	transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);

	// remove any periods from the header name
	std::replace(headerName.begin(), headerName.end(), '.', '_');

	// Add header stop
	strm << "#if !defined(_" << HeaderNamespace <<  headerName << "_)" << endl;

	strm << "#define _" << HeaderNamespace << headerName << "_" << endl << endl;

	// Removed from loop
	strm << "namespace _REFLECTION_" << HeaderNamespace << headerName << "_" << " {" << endl;

	if (EnableHotswap)
		strm << tabchar << "RTTR_PLUGIN_REGISTRATION" << endl;
	else
		strm << tabchar << "RTTR_REGISTRATION" << endl;

	strm << tabchar << "{" << endl;

	// end loop removal

	for (auto item : Definitions) 
	{
		// Exclude child elements
		if (!item.ParentClass.empty() || item.EntryName.empty())
			continue;

		string level = GetAccessString(item.AccessLevel);

		if(!level.empty()) {
			level = ", " + level;
		}

		switch (item.Type)
		{
		case ReflectionType::ENUM:
		case ReflectionType::CLASS:
			if (item.Type == CLASS)
				strm << tabchar << tabchar << "rttr::registration::class_<" << item.EntryName << ">" << "(" << QuoteString(Classpath + item.EntryName) << ")" << endl;
			else
				strm << tabchar << tabchar << "rttr::registration::enumeration<" << item.EntryName << ">" << "(" << QuoteString(Classpath + item.EntryName) << ")" << endl;

			WriteMetaData(item);

			WriteConstructors(item);

			WriteSubProps(item);

			break;
			case PROPERTY:
				if (item.ParentClass.empty())
				{
					string NewEntry = item.EntryName;
					string HasSetter;

					if(item.IsAccessor) {
						HasSetter = GetSetFunction(NewEntry);
						// If this was empty lets just leave it at that
						if(!HasSetter.empty()) {
							HasSetter = ", &" + HasSetter;
							NewEntry = NewEntry.erase(0, 3);
						}
					}

					strm << tabchar << tabchar << "rttr::registration::property(" << QuoteString(NewEntry) << ", " << "&" << item.EntryName << HasSetter << level << ")" << endl;
					WriteMetaData(item);
				}
				break;
			case FUNCTION:
				if (item.ParentClass.empty())
				{
					strm << tabchar << tabchar << "rttr::registration::method(" << QuoteString(item.EntryName) << ", " << "&" << item.EntryName << level << ")" << endl;
					WriteMetaData(item);
				}
			break;
		}

		strm << tabchar << tabchar << ";" << endl;
	}
	// Removed from loop
	strm << tabchar << "}" << endl;
	strm << "};" << endl;

	strm << endl;

	strm << "#endif";
	// end loop removal

	strm.flush();
	strm.close();
}

void RTTRGenerator::WriteFunctionParameters(ReflectionDefinition &item)
{
    if (!item.ParamNames.empty())
    {
        strm << tabchar << tabchar << tabchar << "rttr::parameter_names(";
        for (int i = 0; i < item.ParamNames.size(); i++)
        {
            strm << QuoteString(item.ParamNames[i]);

            if (i + 1 < item.ParamNames.size())
                    strm << ", ";
        }

        strm << ")" << ((item.Metadata.empty() && item.Values.empty()) ? "" : ", ") << endl;
    }
}

void RTTRGenerator::WriteEnumValues(ReflectionDefinition& item)
{
    map<string, string>::iterator current = item.Values.begin();
	map<string, string>::iterator last = item.Values.end();

	while (current != last)
	{
		strm << tabchar << tabchar << tabchar << "rttr::value(" << QuoteString((*current).first) << ", " << item.EntryName << "::" << (*current).second << ")";

		// if this is not the last item
		if (++current != last || !item.Metadata.empty())
			strm << ", ";

		strm << endl;
	}
}

void RTTRGenerator::WriteMetaTags(ReflectionDefinition& item)
{
	map<string, string>::iterator current = item.Metadata.begin();
	map<string, string>::iterator last = item.Metadata.end();

	WriteSpecialTags(item);

	while (current != last)
	{
		strm << tabchar << tabchar << tabchar << "rttr::metadata(" << QuoteString((*current).first) << ", ";

		PropertyType pt = GetPropertyType((*current).second);

		switch (pt)
		{
		case STRING:
			strm << "rttr::string_view(" << (*current).second << ")";
			break;
		case NUMBER:
		case OBJECT:
			strm << (*current).second;
		}

		strm << ")";

		// if this is not the last item
		if (++current != last)
			strm << ", ";

		strm << endl;
	}
}

void RTTRGenerator::WriteMetaData(ReflectionDefinition& item)
{
    if (!item.Metadata.empty() || !item.Values.empty())
	{
		strm << tabchar << tabchar << "(" << endl;

		WriteFunctionParameters(item);

		WriteEnumValues(item);

		WriteMetaTags(item);

		strm << tabchar << tabchar << ")" << endl;
	}
}

void RTTRGenerator::WriteSubProps(ReflectionDefinition& item)
{
    vector<ReflectionDefinition> subProps;

	// Find members of this class with reflection info
	for (int i = 0; i < Definitions.size(); i++)
	{
		if (Definitions[i].ParentClass == item.EntryName && Definitions[i].Type != item.Type)
			subProps.push_back(Definitions[i]);
	}

	for (auto prop : subProps)
	{
		string level = GetAccessString(prop.AccessLevel);

		if(!level.empty()) {
			level = ", " + level;
		}

		if (prop.Type == FUNCTION)
		{
			strm << tabchar << tabchar << ".method(\"" << prop.EntryName << "\", &" << prop.ParentClass << "::" << prop.EntryName << level << ")" << endl;
		}
		else if (prop.Type == PROPERTY)
		{
			string NewEntry = prop.EntryName;
			string HasSetter;

			if(prop.IsAccessor) {
				HasSetter = GetSetFunction(NewEntry);
				// If this was empty lets just leave it at that
				if(!HasSetter.empty()) {
					HasSetter = ", &" + prop.ParentClass + "::" + HasSetter;
					NewEntry = NewEntry.erase(0, 3);
				}
			}

			strm << tabchar << tabchar << ".property(" << QuoteString(NewEntry) << ", &" << prop.ParentClass << "::" << prop.EntryName << HasSetter << level << ")" << endl;
		}
		else if (prop.Type == ENUM)
		{
			strm << tabchar << tabchar << ".enumeration<" << prop.EntryName << ">(" << QuoteString(prop.EntryName) << ")" << endl;
		}

		if (!prop.Metadata.empty() || !prop.Values.empty() || !prop.ParamNames.empty() || !prop.DefaultArgs.empty())
		{
			strm << tabchar << tabchar << "(" << endl;

			WriteFunctionParameters(prop);

			WriteEnumValues(prop);

			WriteMetaTags(prop);

			strm << tabchar << tabchar << ")" << endl;
		}
	}
}

void RTTRGenerator::WriteConstructors(ReflectionDefinition& item)
{
    if (item.Type != ENUM) {
		strm << tabchar << tabchar << ".constructor<>()" << endl;

		for (auto& ctorString : item.Constructors)
		{
			strm << tabchar << tabchar << ".constructor<" << ctorString << ">()" << endl;
		}
	}
}

void RTTRGenerator::WriteSpecialTags(ReflectionDefinition& item)
{
	map<string, string>::iterator current = item.Metadata.begin();
	map<string, string>::iterator last = item.Metadata.end();

	// Validate and write return policy
	if (item.Type != ENUM)
	{
		if (!item.ReturnPolicy.empty())
		{
			string policy = ValidatePolicy(item);

			if (!policy.empty())
			{
				if (item.Type == FUNCTION)
				{
					strm << tabchar << tabchar << tabchar << "rttr::policy::meth::" << policy;
				}
				else if (item.Type == PROPERTY)
				{
					strm << tabchar << tabchar << tabchar << "rttr::policy::prop::" << policy;
				}
				else if (item.Type == CLASS)
				{
					cout << "Implement Policy for class .Ctors" << endl;
				}

				if (current != last || !item.DefaultArgs.empty())
					strm << ",";

				strm << endl;
			}
		}
	}

	// Write default arguments
	if (item.Type == FUNCTION)
	{
		if (!item.DefaultArgs.empty())
		{
			strm << tabchar << tabchar << tabchar << "rttr::default_arguments(" << item.DefaultArgs << ")";

			if (current != last)
				strm << ",";

			strm << endl;
		}
	}
}