/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy, ronag89@gmail.com
*/

#include "stdafx.h"

#include "env.h"

#include "../version.h"

#include "except.h"
#include "log.h"
#include "string.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/once.hpp>

#include <functional>
#include <iostream>
#include <fstream>

namespace caspar { namespace env {
	
std::wstring media;
std::wstring log;
std::wstring ftemplate;
std::wstring data;
std::wstring font;
std::wstring thumbnails;
boost::property_tree::wptree pt;

void check_is_configured()
{
	if(pt.empty())
		CASPAR_THROW_EXCEPTION(invalid_operation() << msg_info(L"Enviroment properties has not been configured"));
}

void configure(const std::wstring& filename)
{
	try
	{
		auto initialPath = boost::filesystem::initial_path().wstring();
	
		boost::filesystem::wifstream file(initialPath + L"/" + filename);
		boost::property_tree::read_xml(file, pt, boost::property_tree::xml_parser::trim_whitespace | boost::property_tree::xml_parser::no_comments);

		auto paths	= pt.get_child(L"configuration.paths");
		media		= paths.get(L"media-path", initialPath + L"/media/");
		log			= paths.get(L"log-path", initialPath + L"/log/");
		ftemplate	= boost::filesystem::complete(paths.get(L"template-path", initialPath + L"/template/")).wstring();
		data		= paths.get(L"data-path", initialPath + L"/data/");
		font		= paths.get(L"font-path", initialPath + L"/fonts/");
		thumbnails	= paths.get(L"thumbnails-path", initialPath + L"/data/");

		//Make sure that all paths have a trailing backslash
		if(media.at(media.length()-1) != L'/')
			media.append(L"/");
		if(log.at(log.length()-1) != L'/')
			log.append(L"/");
		if(ftemplate.at(ftemplate.length()-1) != L'/')
			ftemplate.append(L"/");
		if(data.at(data.length()-1) != L'/')
			data.append(L"/");
		if(font.at(font.length()-1) != L'/')
			font.append(L"/");
		if(thumbnails.at(thumbnails.length()-1) != L'/')
			thumbnails.append(L"/");

		try
		{
			for(auto it = boost::filesystem::directory_iterator(initialPath); it != boost::filesystem::directory_iterator(); ++it)
			{
				if(it->path().wstring().find(L".fth") != std::wstring::npos)			
				{
					auto from_path = *it;
					auto to_path = boost::filesystem::path(ftemplate + L"/" + it->path().wstring());
				
					if(boost::filesystem::exists(to_path))
						boost::filesystem::remove(to_path);

					boost::filesystem::copy_file(from_path, to_path);
				}	
			}
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
			CASPAR_LOG(error) << L"Failed to copy template-hosts from initial-path to template-path.";
		}
	}
	catch(...)
	{
		CASPAR_LOG(error) << L" ### Invalid configuration file. ###";
		throw;
	}

	try
	{
		try
		{
			auto log_path = boost::filesystem::path(log);
			if(!boost::filesystem::exists(log_path))
				boost::filesystem::create_directories(log_path);
		}
		catch(...)
		{
			log = L"./";
		}

		auto media_path = boost::filesystem::path(media);
		if(!boost::filesystem::exists(media_path))
			boost::filesystem::create_directories(media_path);
				
		auto template_path = boost::filesystem::path(ftemplate);
		if(!boost::filesystem::exists(template_path))
			boost::filesystem::create_directories(template_path);
		
		auto data_path = boost::filesystem::path(data);
		if(!boost::filesystem::exists(data_path))
			boost::filesystem::create_directories(data_path);

		auto font_path = boost::filesystem::path(font);
		if(!boost::filesystem::exists(font_path))
			boost::filesystem::create_directories(font_path);

		auto thumbnails_path = boost::filesystem::path(thumbnails);
		if(!boost::filesystem::exists(thumbnails_path))
			boost::filesystem::create_directories(thumbnails_path);
	}
	catch(...)
	{
		CASPAR_LOG_CURRENT_EXCEPTION();
		CASPAR_LOG(error) << L"Failed to create configured directories.";
	}
}
	
const std::wstring& media_folder()
{
	check_is_configured();
	return media;
}

const std::wstring& log_folder()
{
	check_is_configured();
	return log;
}

const std::wstring& template_folder()
{
	check_is_configured();
	return ftemplate;
}

const std::wstring& data_folder()
{
	check_is_configured();
	return data;
}

const std::wstring& font_folder()
{
	check_is_configured();
	return font;
}

const std::wstring& thumbnails_folder()
{
	check_is_configured();
	return thumbnails;
}

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

const std::wstring& version()
{
	static std::wstring ver = u16(
			EXPAND_AND_QUOTE(CASPAR_GEN)	"." 
			EXPAND_AND_QUOTE(CASPAR_MAYOR)  "." 
			EXPAND_AND_QUOTE(CASPAR_MINOR)  "." 
			CASPAR_REV	" " 
			CASPAR_TAG);
	return ver;
}

const boost::property_tree::wptree& properties()
{
	check_is_configured();
	return pt;
}

std::wstring system_font_folder()
{
	return L"C:\\windows\\Fonts\\";
}

}}
