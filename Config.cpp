
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"

std::map<std::string,std::string>	Config::statusCodeMessages;
std::map<std::string, std::string> 	Config::mimetypeMap;

Config::Config(std::string file) : file_name(file) , full_file(), lines(), servers()
{
    init_statusCodeMessages();
	init_mimetype();
}

Config::~Config(){}

Config::Config(Config const& c) 
{
    *this = c;
}

Config &Config::operator=(Config const& c) 
{
    servers = c.servers;
    lines = c.lines;
    _binders = c._binders;
    init_statusCodeMessages();
	init_mimetype();
    return *this;
}

std::vector<Server> Config::getServers() const
{
    return servers;
}

std::string Config::getFullFile() const
{
    return full_file;
}

std::vector<std::string> Config::getLines() const
{
    return lines;
}
std::string Config::getLine(std::string &) const
{
    return "sever_name";
}

Server Config::getServer(int port, std::string &c) const
{
    std::vector<Server> a;
    int save = 0;
    int j = 0;
    int check = 0;
    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].getPort() == port)
            a.push_back(servers[i]);
    }
    if (a.size() == 1)
        return a[0];
    if (c == "127.0.0.1" && a.size() == 0)
        return servers[0];
    if (c == "127.0.0.1")
        return a[0];
    else if (!c.size())
            throw NoServerFoundException();
    if (a.size() > 1)
    {
        for (size_t i = 0; i < a.size(); i++)
        {
            if (a[i].checkServerName(c))
            {
                save = i;
                j++;
            }
        }
    }
    else
    {
        check = 1;
        for (size_t i = 0; i < servers.size(); i++)
        {
            if (servers[i].checkServerName(c))
            {
                save = i;
                j++;
            }
    
        }
    }
    if (j > 1)
        throw (MultpiteServerName());
    if (check == 0)
        return a[save];
    return servers[save];
}

void Config::getServer2(int port )
{
    std::vector<Server> a;
    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].getPort() == port)
            a.push_back(servers[i]);
    }
    if (a.size() == 1)
        return ;
    if (a.size() > 1)
    {
        for (size_t i = 0; i < a.size() - 1; i++)
        {
            for (size_t j = i + 1; j < a.size(); j++)
            {
                if (a[i].checkServerName1(a[j].getServerName()))
                    throw (MultpiteServerName());
    
            }
        }
    }
}

 std::map<int, std::string> Config::getBinders() const
 {
        return _binders;
 }

void Config::setfile(std::string& file)
{
    full_file = file;
}

void Config::main_read()
{
    read_file();
    split_line();
    split_servers();
    main_error_check();
    parse_server();
    parse_bind_map();
    checkServerNamess();
}

void Config::read_file()
{
    std::fstream a(file_name);

    if (!a.is_open())
    {
        std::cout << "Error : " << file_name;
        throw fileErrorException();
    }
    std::string output;
    while (std::getline(a, output, '\t'))
        full_file += output;
}

void    Config::split_line()
{
    std::string line;
    std::string full_file1(full_file);
    size_t pos = 0;
    while ((pos = full_file1.find("\n")) != std::string::npos)
    {
        line = full_file1.substr(0, pos);
        if (line[0] != '\0')
        lines.push_back(line);
        full_file1.erase(0, pos + 1);
    }
    lines.push_back(line);
    full_file1.erase(0, pos + 1);
}

void Config::split_servers()
{
    int open = 1;
    Server  s1;
    std::vector<std::string>::iterator it = lines.begin();
    std::vector<std::string>::iterator save = it;
    while (it != lines.end())
    {
        if ((*it).find("server ") != std::string::npos && open == 1)
            open = 0;
        else if ((*it).find("server ") != std::string::npos && open == 0)
        {
            std::vector<std::string> a (save, it);
            s1.setServerline(a);
            servers.push_back(s1);
            save = it;
        }
        it++;
    }
    std::vector<std::string> a (save, it);
    s1.setServerline(a);
    servers.push_back(s1);
}

void Config::parse_server()
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        servers[i].parseLines();
        servers[i].checkRootloc();
    }
}

void Config::parse_bind_map()
{
    for (size_t i = 0; i < servers.size(); i++)
        _binders.insert(std::pair<int, std::string>(servers[i].getPort(),servers[i].getIp()));
}

void Config::main_error_check()
{
    check_brekets();
}

void Config::check_brekets()
{
    int open;
    open = 0;
    for (int i = 0; full_file[i]; i++)
    {
        if (full_file[i] == '{')
            open++;
        if (full_file[i] == '}')
            open--;
    }
    if (open != 0)
        throw NotclosedBrackets();
}

void Config::checkServerNamess()
{
    std::map<int, std::string>::iterator it = _binders.begin();
    for (; it != _binders.end(); ++it)
    {
        try
        {
            getServer2(it->first);
        }
        catch(const char * e)
        {
            std::cerr << e << '\n';
        }
    }
}

void Config::init_statusCodeMessages()//TODO put it in a better place
	{
		statusCodeMessages["100"] = "Continue";
		statusCodeMessages["200"] = "OK";
		statusCodeMessages["201"] = "Created";
		statusCodeMessages["202"] = "Accepted";
		statusCodeMessages["203"] = "Non-Authoritative Information";
		statusCodeMessages["204"] = "No Content";
		statusCodeMessages["205"] = "Reset Content";
		statusCodeMessages["206"] = "Partial Content";
		statusCodeMessages["300"] = "Multiple Choices";
		statusCodeMessages["301"] = "Moved Permanently";
		statusCodeMessages["302"] = "Found";
		statusCodeMessages["303"] = "See Other";
		statusCodeMessages["304"] = "Not Modified";
		statusCodeMessages["305"] = "Use Proxy";
		statusCodeMessages["307"] = "Temporary Redirect";
		statusCodeMessages["400"] = "Bad Request";
		statusCodeMessages["401"] = "Unauthorized";
		statusCodeMessages["403"] = "Forbidden";
		statusCodeMessages["404"] = "Not Found";
		statusCodeMessages["405"] = "Method Not Allowed";
		statusCodeMessages["406"] = "Not Acceptable";
		statusCodeMessages["407"] = "Proxy Authentication Required";
		statusCodeMessages["408"] = "Request Time-out";
		statusCodeMessages["409"] = "Conflict";
		statusCodeMessages["410"] = "Gone";
		statusCodeMessages["411"] = "Length Required";
		statusCodeMessages["412"] = "Precondition Failed";
		statusCodeMessages["413"] = "Request Entity Too Large";
		statusCodeMessages["414"] = "Request-URI Too Large";
		statusCodeMessages["415"] = "Unsupported Media Type";
		statusCodeMessages["416"] = "Requested range not satisfiable";
		statusCodeMessages["417"] = "Expectation Failed";
		statusCodeMessages["500"] = "Internal Server Error";
		statusCodeMessages["501"] = "Not Implemented";
		statusCodeMessages["502"] = "Bad Gateway";
		statusCodeMessages["503"] = "Service Unavailable";
		statusCodeMessages["504"] = "Gateway Time-out";
		statusCodeMessages["505"] = "HTTP Version not supported";
	}
	void Config::init_mimetype()
	{
		mimetypeMap["txt"]= "text/plain";
        mimetypeMap["pdf"]= "application/pdf";
        mimetypeMap["html"]= "text/html";
        mimetypeMap["htm"]= "text/html";
        mimetypeMap["xml"]= "text/xml";
        mimetypeMap["js"]= "application/x-javascript";
        mimetypeMap["xhtml"]= "application/xhtml+xml";
        mimetypeMap["svg"]= "image/svg+xml";
        mimetypeMap["svgz"]= "image/svg+xml";
        mimetypeMap["jpg"]= "image/jpeg";
        mimetypeMap["jpeg"]= "image/jpeg";
        mimetypeMap["png"]= "image/png";
        mimetypeMap["tif"]= "image/tiff";
        mimetypeMap["tiff"]= "image/tiff";
        mimetypeMap["ico"]= "image/ico";
        mimetypeMap["cur"]= "image/ico";
        mimetypeMap["bmp"]= "image/bmp";
        mimetypeMap["wml"]= "text/vnd.wap.wml";
        mimetypeMap["wmlc"]= "application/vnd.wap.wmlc";
        mimetypeMap["323"] = "text/h323";
	    mimetypeMap["3g2"] = "video/3gpp2";
	    mimetypeMap["3gp"] = "video/3gpp";
	    mimetypeMap["3gp2"] = "video/3gpp2";
	    mimetypeMap["3gpp"] = "video/3gpp";
	    mimetypeMap["7z"] = "application/x-7z-compressed";
	    mimetypeMap["aa"] = "audio/audible";
	    mimetypeMap["AAC"] = "audio/aac";
	    mimetypeMap["aaf"] = "application/octet-stream";
	    mimetypeMap["aax"] = "audio/vnd.audible.aax";
	    mimetypeMap["ac3"] = "audio/ac3";
	    mimetypeMap["aca"] = "application/octet-stream";
	    mimetypeMap["accda"] = "application/msaccess.addin";
	    mimetypeMap["accdb"] = "application/msaccess";
	    mimetypeMap["accdc"] = "application/msaccess.cab";
	    mimetypeMap["accde"] = "application/msaccess";
	    mimetypeMap["accdr"] = "application/msaccess.runtime";
	    mimetypeMap["accdt"] = "application/msaccess";
	    mimetypeMap["accdw"] = "application/msaccess.webapplication";
	    mimetypeMap["accft"] = "application/msaccess.ftemplate";
	    mimetypeMap["acx"] = "application/internet-property-stream";
	    mimetypeMap["AddIn"] = "text/xml";
	    mimetypeMap["ade"] = "application/msaccess";
	    mimetypeMap["adobebridge"] = "application/x-bridge-url";
	    mimetypeMap["adp"] = "application/msaccess";
	    mimetypeMap["ADT"] = "audio/vnd.dlna.adts";
	    mimetypeMap["ADTS"] = "audio/aac";
	    mimetypeMap["afm"] = "application/octet-stream";
	    mimetypeMap["ai"] = "application/postscript";
	    mimetypeMap["aif"] = "audio/aiff";
	    mimetypeMap["aifc"] = "audio/aiff";
	    mimetypeMap["aiff"] = "audio/aiff";
	    mimetypeMap["air"] = "application/vnd.adobe.air-application-installer-package+zip";
	    mimetypeMap["amc"] = "application/mpeg";
	    mimetypeMap["anx"] = "application/annodex";
	    mimetypeMap["apk"] = "application/vnd.android.package-archive";
	    mimetypeMap["apng"] = "image/apng";
	    mimetypeMap["application"] = "application/x-ms-application";
	    mimetypeMap["art"] = "image/x-jg";
	    mimetypeMap["asa"] = "application/xml";
	    mimetypeMap["asax"] = "application/xml";
	    mimetypeMap["ascx"] = "application/xml";
	    mimetypeMap["asd"] = "application/octet-stream";
	    mimetypeMap["asf"] = "video/x-ms-asf";
	    mimetypeMap["ashx"] = "application/xml";
	    mimetypeMap["asi"] = "application/octet-stream";
	    mimetypeMap["asm"] = "text/plain";
	    mimetypeMap["asmx"] = "application/xml";
	    mimetypeMap["aspx"] = "application/xml";
	    mimetypeMap["asr"] = "video/x-ms-asf";
	    mimetypeMap["asx"] = "video/x-ms-asf";
	    mimetypeMap["atom"] = "application/atom+xml";
	    mimetypeMap["au"] = "audio/basic";
	    mimetypeMap["avci"] = "image/avci";
	    mimetypeMap["avcs"] = "image/avcs";
	    mimetypeMap["avi"] = "video/x-msvideo";
	    mimetypeMap["avif"] = "image/avif";
	    mimetypeMap["avifs"] = "image/avif-sequence";
	    mimetypeMap["axa"] = "audio/annodex";
	    mimetypeMap["axs"] = "application/olescript";
	    mimetypeMap["axv"] = "video/annodex";
	    mimetypeMap["bas"] = "text/plain";
	    mimetypeMap["bcpio"] = "application/x-bcpio";
	    mimetypeMap["bin"] = "application/octet-stream";
	    mimetypeMap["bmp"] = "image/bmp";
	    mimetypeMap["c"] = "text/plain";
	    mimetypeMap["cab"] = "application/octet-stream";
	    mimetypeMap["caf"] = "audio/x-caf";
	    mimetypeMap["calx"] = "application/vnd.ms-office.calx";
	    mimetypeMap["cat"] = "application/vnd.ms-pki.seccat";
	    mimetypeMap["cc"] = "text/plain";
	    mimetypeMap["cd"] = "text/plain";
	    mimetypeMap["cdda"] = "audio/aiff";
	    mimetypeMap["cdf"] = "application/x-cdf";
	    mimetypeMap["cer"] = "application/x-x509-ca-cert";
	    mimetypeMap["cfg"] = "text/plain";
	    mimetypeMap["chm"] = "application/octet-stream";
	    mimetypeMap["class"] = "application/x-java-applet";
	    mimetypeMap["clp"] = "application/x-msclip";
	    mimetypeMap["cmd"] = "text/plain";
	    mimetypeMap["cmx"] = "image/x-cmx";
	    mimetypeMap["cnf"] = "text/plain";
	    mimetypeMap["cod"] = "image/cis-cod";
	    mimetypeMap["config"] = "application/xml";
	    mimetypeMap["contact"] = "text/x-ms-contact";
	    mimetypeMap["coverage"] = "application/xml";
	    mimetypeMap["cpio"] = "application/x-cpio";
	    mimetypeMap["cpp"] = "text/plain";
	    mimetypeMap["crd"] = "application/x-mscardfile";
	    mimetypeMap["crl"] = "application/pkix-crl";
	    mimetypeMap["crt"] = "application/x-x509-ca-cert";
	    mimetypeMap["cs"] = "text/plain";
	    mimetypeMap["csdproj"] = "text/plain";
	    mimetypeMap["csh"] = "application/x-csh";
	    mimetypeMap["csproj"] = "text/plain";
	    mimetypeMap["css"] = "text/css";
	    mimetypeMap["csv"] = "text/csv";
	    mimetypeMap["cur"] = "application/octet-stream";
	    mimetypeMap["czx"] = "application/x-czx";
	    mimetypeMap["cxx"] = "text/plain";
	    mimetypeMap["dat"] = "application/octet-stream";
	    mimetypeMap["datasource"] = "application/xml";
	    mimetypeMap["dbproj"] = "text/plain";
	    mimetypeMap["dcr"] = "application/x-director";
	    mimetypeMap["def"] = "text/plain";
	    mimetypeMap["deploy"] = "application/octet-stream";
	    mimetypeMap["der"] = "application/x-x509-ca-cert";
	    mimetypeMap["dgml"] = "application/xml";
	    mimetypeMap["dib"] = "image/bmp";
	    mimetypeMap["dif"] = "video/x-dv";
	    mimetypeMap["dir"] = "application/x-director";
	    mimetypeMap["disco"] = "text/xml";
	    mimetypeMap["divx"] = "video/divx";
	    mimetypeMap["dll"] = "application/x-msdownload";
	    mimetypeMap["dll.config"] = "text/xml";
	    mimetypeMap["dlm"] = "text/dlm";
	    mimetypeMap["doc"] = "application/msword";
	    mimetypeMap["docm"] = "application/vnd.ms-word.document.macroEnabled.12";
	    mimetypeMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	    mimetypeMap["dot"] = "application/msword";
	    mimetypeMap["dotm"] = "application/vnd.ms-word.template.macroEnabled.12";
	    mimetypeMap["dotx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.template";
	    mimetypeMap["dsp"] = "application/octet-stream";
	    mimetypeMap["dsw"] = "text/plain";
	    mimetypeMap["dtd"] = "text/xml";
	    mimetypeMap["dtsConfig"] = "text/xml";
	    mimetypeMap["dv"] = "video/x-dv";
	    mimetypeMap["dvi"] = "application/x-dvi";
	    mimetypeMap["dwf"] = "drawing/x-dwf";
	    mimetypeMap["dwg"] = "application/acad";
	    mimetypeMap["dwp"] = "application/octet-stream";
	    mimetypeMap["dxf"] = "application/x-dxf";
	    mimetypeMap["dxr"] = "application/x-director";
	    mimetypeMap["eml"] = "message/rfc822";
	    mimetypeMap["emf"] = "image/emf";
	    mimetypeMap["emz"] = "application/octet-stream";
	    mimetypeMap["eot"] = "application/vnd.ms-fontobject";
	    mimetypeMap["eps"] = "application/postscript";
	    mimetypeMap["es"] = "application/ecmascript";
	    mimetypeMap["etl"] = "application/etl";
	    mimetypeMap["etx"] = "text/x-setext";
	    mimetypeMap["evy"] = "application/envoy";
	    mimetypeMap["exe"] = "application/vnd.microsoft.portable-executable";
	    mimetypeMap["exe.config"] = "text/xml";
	    mimetypeMap["f4v"] = "video/mp4";
	    mimetypeMap["fdf"] = "application/vnd.fdf";
	    mimetypeMap["fif"] = "application/fractals";
	    mimetypeMap["filters"] = "application/xml";
	    mimetypeMap["fla"] = "application/octet-stream";
	    mimetypeMap["flac"] = "audio/flac";
	    mimetypeMap["flr"] = "x-world/x-vrml";
	    mimetypeMap["flv"] = "video/x-flv";
	    mimetypeMap["fsscript"] = "application/fsharp-script";
	    mimetypeMap["fsx"] = "application/fsharp-script";
	    mimetypeMap["generictest"] = "application/xml";
	    mimetypeMap["geojson"] = "application/geo+json";
	    mimetypeMap["gif"] = "image/gif";
	    mimetypeMap["gpx"] = "application/gpx+xml";
	    mimetypeMap["group"] = "text/x-ms-group";
	    mimetypeMap["gsm"] = "audio/x-gsm";
	    mimetypeMap["gtar"] = "application/x-gtar";
	    mimetypeMap["gz"] = "application/x-gzip";
	    mimetypeMap["h"] = "text/plain";
	    mimetypeMap["hdf"] = "application/x-hdf";
	    mimetypeMap["hdml"] = "text/x-hdml";
	    mimetypeMap["heic"] = "image/heic";
	    mimetypeMap["heics"] = "image/heic-sequence";
	    mimetypeMap["heif"] = "image/heif";
	    mimetypeMap["heifs"] = "image/heif-sequence";
	    mimetypeMap["hhc"] = "application/x-oleobject";
	    mimetypeMap["hhk"] = "application/octet-stream";
	    mimetypeMap["hhp"] = "application/octet-stream";
	    mimetypeMap["hlp"] = "application/winhlp";
	    mimetypeMap["hpp"] = "text/plain";
	    mimetypeMap["hqx"] = "application/mac-binhex40";
	    mimetypeMap["hta"] = "application/hta";
	    mimetypeMap["htc"] = "text/x-component";
	    mimetypeMap["htm"] = "text/html";
	    mimetypeMap["html"] = "text/html";
	    mimetypeMap["htt"] = "text/webviewhtml";
	    mimetypeMap["hxa"] = "application/xml";
	    mimetypeMap["hxc"] = "application/xml";
	    mimetypeMap["hxd"] = "application/octet-stream";
	    mimetypeMap["hxe"] = "application/xml";
	    mimetypeMap["hxf"] = "application/xml";
	    mimetypeMap["hxh"] = "application/octet-stream";
	    mimetypeMap["hxi"] = "application/octet-stream";
	    mimetypeMap["hxk"] = "application/xml";
	    mimetypeMap["hxq"] = "application/octet-stream";
	    mimetypeMap["hxr"] = "application/octet-stream";
	    mimetypeMap["hxs"] = "application/octet-stream";
	    mimetypeMap["hxt"] = "text/html";
	    mimetypeMap["hxv"] = "application/xml";
	    mimetypeMap["hxw"] = "application/octet-stream";
	    mimetypeMap["hxx"] = "text/plain";
	    mimetypeMap["i"] = "text/plain";
	    mimetypeMap["ical"] = "text/calendar";
	    mimetypeMap["icalendar"] = "text/calendar";
	    mimetypeMap["ico"] = "image/x-icon";
	    mimetypeMap["ics"] = "text/calendar";
	    mimetypeMap["idl"] = "text/plain";
	    mimetypeMap["ief"] = "image/ief";
	    mimetypeMap["ifb"] = "text/calendar";
	    mimetypeMap["iii"] = "application/x-iphone";
	    mimetypeMap["inc"] = "text/plain";
	    mimetypeMap["inf"] = "application/octet-stream";
	    mimetypeMap["ini"] = "text/plain";
	    mimetypeMap["inl"] = "text/plain";
	    mimetypeMap["ins"] = "application/x-internet-signup";
	    mimetypeMap["ipa"] = "application/x-itunes-ipa";
	    mimetypeMap["ipg"] = "application/x-itunes-ipg";
	    mimetypeMap["ipproj"] = "text/plain";
	    mimetypeMap["ipsw"] = "application/x-itunes-ipsw";
	    mimetypeMap["iqy"] = "text/x-ms-iqy";
	    mimetypeMap["isp"] = "application/x-internet-signup";
	    mimetypeMap["isma"] = "application/octet-stream";
	    mimetypeMap["ismv"] = "application/octet-stream";
	    mimetypeMap["ite"] = "application/x-itunes-ite";
	    mimetypeMap["itlp"] = "application/x-itunes-itlp";
	    mimetypeMap["itms"] = "application/x-itunes-itms";
	    mimetypeMap["itpc"] = "application/x-itunes-itpc";
	    mimetypeMap["IVF"] = "video/x-ivf";
	    mimetypeMap["jar"] = "application/java-archive";
	    mimetypeMap["java"] = "application/octet-stream";
	    mimetypeMap["jck"] = "application/liquidmotion";
	    mimetypeMap["jcz"] = "application/liquidmotion";
	    mimetypeMap["jfif"] = "image/pjpeg";
	    mimetypeMap["jnlp"] = "application/x-java-jnlp-file";
	    mimetypeMap["jpb"] = "application/octet-stream";
	    mimetypeMap["jpe"] = "image/jpeg";
	    mimetypeMap["jpeg"] = "image/jpeg";
	    mimetypeMap["jpg"] = "image/jpeg";
	    mimetypeMap["js"] = "application/javascript";
	    mimetypeMap["json"] = "application/json";
	    mimetypeMap["jsx"] = "text/jscript";
	    mimetypeMap["jsxbin"] = "text/plain";
	    mimetypeMap["key"] = "application/vnd.apple.keynote";
	    mimetypeMap["latex"] = "application/x-latex";
	    mimetypeMap["library-ms"] = "application/windows-library+xml";
	    mimetypeMap["lit"] = "application/x-ms-reader";
	    mimetypeMap["loadtest"] = "application/xml";
	    mimetypeMap["lpk"] = "application/octet-stream";
	    mimetypeMap["lsf"] = "video/x-la-asf";
	    mimetypeMap["lst"] = "text/plain";
	    mimetypeMap["lsx"] = "video/x-la-asf";
	    mimetypeMap["lzh"] = "application/octet-stream";
	    mimetypeMap["m13"] = "application/x-msmediaview";
	    mimetypeMap["m14"] = "application/x-msmediaview";
	    mimetypeMap["m1v"] = "video/mpeg";
	    mimetypeMap["m2t"] = "video/vnd.dlna.mpeg-tts";
	    mimetypeMap["m2ts"] = "video/vnd.dlna.mpeg-tts";
	    mimetypeMap["m2v"] = "video/mpeg";
	    mimetypeMap["m3u"] = "audio/x-mpegurl";
	    mimetypeMap["m3u8"] = "audio/x-mpegurl";
	    mimetypeMap["m4a"] = "audio/m4a";
	    mimetypeMap["m4b"] = "audio/m4b";
	    mimetypeMap["m4p"] = "audio/m4p";
	    mimetypeMap["m4r"] = "audio/x-m4r";
	    mimetypeMap["m4v"] = "video/x-m4v";
	    mimetypeMap["mac"] = "image/x-macpaint";
	    mimetypeMap["mak"] = "text/plain";
	    mimetypeMap["man"] = "application/x-troff-man";
	    mimetypeMap["manifest"] = "application/x-ms-manifest";
	    mimetypeMap["map"] = "text/plain";
	    mimetypeMap["master"] = "application/xml";
	    mimetypeMap["mbox"] = "application/mbox";
	    mimetypeMap["mda"] = "application/msaccess";
	    mimetypeMap["mdb"] = "application/x-msaccess";
	    mimetypeMap["mde"] = "application/msaccess";
	    mimetypeMap["mdp"] = "application/octet-stream";
	    mimetypeMap["me"] = "application/x-troff-me";
	    mimetypeMap["mfp"] = "application/x-shockwave-flash";
	    mimetypeMap["mht"] = "message/rfc822";
	    mimetypeMap["mhtml"] = "message/rfc822";
	    mimetypeMap["mid"] = "audio/mid";
	    mimetypeMap["midi"] = "audio/mid";
	    mimetypeMap["mix"] = "application/octet-stream";
	    mimetypeMap["mk"] = "text/plain";
	    mimetypeMap["mk3d"] = "video/x-matroska-3d";
	    mimetypeMap["mka"] = "audio/x-matroska";
	    mimetypeMap["mkv"] = "video/x-matroska";
	    mimetypeMap["mmf"] = "application/x-smaf";
	    mimetypeMap["mno"] = "text/xml";
	    mimetypeMap["mny"] = "application/x-msmoney";
	    mimetypeMap["mod"] = "video/mpeg";
	    mimetypeMap["mov"] = "video/quicktime";
	    mimetypeMap["movie"] = "video/x-sgi-movie";
	    mimetypeMap["mp2"] = "video/mpeg";
	    mimetypeMap["mp2v"] = "video/mpeg";
	    mimetypeMap["mp3"] = "audio/mpeg";
	    mimetypeMap["mp4"] = "video/mp4";
	    mimetypeMap["mp4v"] = "video/mp4";
	    mimetypeMap["mpa"] = "video/mpeg";
	    mimetypeMap["mpe"] = "video/mpeg";
	    mimetypeMap["mpeg"] = "video/mpeg";
	    mimetypeMap["mpf"] = "application/vnd.ms-mediapackage";
	    mimetypeMap["mpg"] = "video/mpeg";
	    mimetypeMap["mpp"] = "application/vnd.ms-project";
	    mimetypeMap["mpv2"] = "video/mpeg";
	    mimetypeMap["mqv"] = "video/quicktime";
	    mimetypeMap["ms"] = "application/x-troff-ms";
	    mimetypeMap["msg"] = "application/vnd.ms-outlook";
	    mimetypeMap["msi"] = "application/octet-stream";
	    mimetypeMap["mso"] = "application/octet-stream";
	    mimetypeMap["mts"] = "video/vnd.dlna.mpeg-tts";
	    mimetypeMap["mtx"] = "application/xml";
	    mimetypeMap["mvb"] = "application/x-msmediaview";
	    mimetypeMap["mvc"] = "application/x-miva-compiled";
	    mimetypeMap["mxf"] = "application/mxf";
	    mimetypeMap["mxp"] = "application/x-mmxp";
	    mimetypeMap["nc"] = "application/x-netcdf";
	    mimetypeMap["nsc"] = "video/x-ms-asf";
	    mimetypeMap["numbers"] = "application/vnd.apple.numbers";
	    mimetypeMap["nws"] = "message/rfc822";
	    mimetypeMap["ocx"] = "application/octet-stream";
	    mimetypeMap["oda"] = "application/oda";
	    mimetypeMap["odb"] = "application/vnd.oasis.opendocument.database";
	    mimetypeMap["odc"] = "application/vnd.oasis.opendocument.chart";
	    mimetypeMap["odf"] = "application/vnd.oasis.opendocument.formula";
	    mimetypeMap["odg"] = "application/vnd.oasis.opendocument.graphics";
	    mimetypeMap["odh"] = "text/plain";
	    mimetypeMap["odi"] = "application/vnd.oasis.opendocument.image";
	    mimetypeMap["odl"] = "text/plain";
	    mimetypeMap["odm"] = "application/vnd.oasis.opendocument.text-master";
	    mimetypeMap["odp"] = "application/vnd.oasis.opendocument.presentation";
	    mimetypeMap["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	    mimetypeMap["odt"] = "application/vnd.oasis.opendocument.text";
	    mimetypeMap["oga"] = "audio/ogg";
	    mimetypeMap["ogg"] = "audio/ogg";
	    mimetypeMap["ogv"] = "video/ogg";
	    mimetypeMap["ogx"] = "application/ogg";
	    mimetypeMap["one"] = "application/onenote";
	    mimetypeMap["onea"] = "application/onenote";
	    mimetypeMap["onepkg"] = "application/onenote";
	    mimetypeMap["onetmp"] = "application/onenote";
	    mimetypeMap["onetoc"] = "application/onenote";
	    mimetypeMap["onetoc2"] = "application/onenote";
	    mimetypeMap["opus"] = "audio/ogg";
	    mimetypeMap["orderedtest"] = "application/xml";
	    mimetypeMap["osdx"] = "application/opensearchdescription+xml";
	    mimetypeMap["otf"] = "application/font-sfnt";
	    mimetypeMap["otg"] = "application/vnd.oasis.opendocument.graphics-template";
	    mimetypeMap["oth"] = "application/vnd.oasis.opendocument.text-web";
	    mimetypeMap["otp"] = "application/vnd.oasis.opendocument.presentation-template";
	    mimetypeMap["ots"] = "application/vnd.oasis.opendocument.spreadsheet-template";
	    mimetypeMap["ott"] = "application/vnd.oasis.opendocument.text-template";
	    mimetypeMap["oxps"] = "application/oxps";
	    mimetypeMap["oxt"] = "application/vnd.openofficeorg.extension";
	    mimetypeMap["p10"] = "application/pkcs10";
	    mimetypeMap["p12"] = "application/x-pkcs12";
	    mimetypeMap["p7b"] = "application/x-pkcs7-certificates";
	    mimetypeMap["p7c"] = "application/pkcs7-mime";
	    mimetypeMap["p7m"] = "application/pkcs7-mime";
	    mimetypeMap["p7r"] = "application/x-pkcs7-certreqresp";
	    mimetypeMap["p7s"] = "application/pkcs7-signature";
	    mimetypeMap["pages"] = "application/vnd.apple.pages";
	    mimetypeMap["pbm"] = "image/x-portable-bitmap";
	    mimetypeMap["pcast"] = "application/x-podcast";
	    mimetypeMap["pct"] = "image/pict";
	    mimetypeMap["pcx"] = "application/octet-stream";
	    mimetypeMap["pcz"] = "application/octet-stream";
	    mimetypeMap["pdf"] = "application/pdf";
	    mimetypeMap["pfb"] = "application/octet-stream";
	    mimetypeMap["pfm"] = "application/octet-stream";
	    mimetypeMap["pfx"] = "application/x-pkcs12";
	    mimetypeMap["pgm"] = "image/x-portable-graymap";
	    mimetypeMap["pic"] = "image/pict";
	    mimetypeMap["pict"] = "image/pict";
	    mimetypeMap["pkgdef"] = "text/plain";
	    mimetypeMap["pkgundef"] = "text/plain";
	    mimetypeMap["pko"] = "application/vnd.ms-pki.pko";
	    mimetypeMap["pls"] = "audio/scpls";
	    mimetypeMap["pma"] = "application/x-perfmon";
	    mimetypeMap["pmc"] = "application/x-perfmon";
	    mimetypeMap["pml"] = "application/x-perfmon";
	    mimetypeMap["pmr"] = "application/x-perfmon";
	    mimetypeMap["pmw"] = "application/x-perfmon";
	    mimetypeMap["png"] = "image/png";
	    mimetypeMap["pnm"] = "image/x-portable-anymap";
	    mimetypeMap["pnt"] = "image/x-macpaint";
	    mimetypeMap["pntg"] = "image/x-macpaint";
	    mimetypeMap["pnz"] = "image/png";
	    mimetypeMap["pot"] = "application/vnd.ms-powerpoint";
	    mimetypeMap["potm"] = "application/vnd.ms-powerpoint.template.macroEnabled.12";
	    mimetypeMap["potx"] = "application/vnd.openxmlformats-officedocument.presentationml.template";
	    mimetypeMap["ppa"] = "application/vnd.ms-powerpoint";
	    mimetypeMap["ppam"] = "application/vnd.ms-powerpoint.addin.macroEnabled.12";
	    mimetypeMap["ppm"] = "image/x-portable-pixmap";
	    mimetypeMap["pps"] = "application/vnd.ms-powerpoint";
	    mimetypeMap["ppsm"] = "application/vnd.ms-powerpoint.slideshow.macroEnabled.12";
	    mimetypeMap["ppsx"] = "application/vnd.openxmlformats-officedocument.presentationml.slideshow";
	    mimetypeMap["ppt"] = "application/vnd.ms-powerpoint";
	    mimetypeMap["pptm"] = "application/vnd.ms-powerpoint.presentation.macroEnabled.12";
	    mimetypeMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	    mimetypeMap["prf"] = "application/pics-rules";
	    mimetypeMap["prm"] = "application/octet-stream";
	    mimetypeMap["prx"] = "application/octet-stream";
	    mimetypeMap["ps"] = "application/postscript";
	    mimetypeMap["psc1"] = "application/PowerShell";
	    mimetypeMap["psd"] = "application/octet-stream";
	    mimetypeMap["psess"] = "application/xml";
	    mimetypeMap["psm"] = "application/octet-stream";
	    mimetypeMap["psp"] = "application/octet-stream";
	    mimetypeMap["pst"] = "application/vnd.ms-outlook";
	    mimetypeMap["pub"] = "application/x-mspublisher";
	    mimetypeMap["pwz"] = "application/vnd.ms-powerpoint";
	    mimetypeMap["qht"] = "text/x-html-insertion";
	    mimetypeMap["qhtm"] = "text/x-html-insertion";
	    mimetypeMap["qt"] = "video/quicktime";
	    mimetypeMap["qti"] = "image/x-quicktime";
	    mimetypeMap["qtif"] = "image/x-quicktime";
	    mimetypeMap["qtl"] = "application/x-quicktimeplayer";
	    mimetypeMap["qxd"] = "application/octet-stream";
	    mimetypeMap["ra"] = "audio/x-pn-realaudio";
	    mimetypeMap["ram"] = "audio/x-pn-realaudio";
	    mimetypeMap["rar"] = "application/x-rar-compressed";
	    mimetypeMap["ras"] = "image/x-cmu-raster";
	    mimetypeMap["rat"] = "application/rat-file";
	    mimetypeMap["rc"] = "text/plain";
	    mimetypeMap["rc2"] = "text/plain";
	    mimetypeMap["rct"] = "text/plain";
	    mimetypeMap["rdlc"] = "application/xml";
	    mimetypeMap["reg"] = "text/plain";
	    mimetypeMap["resx"] = "application/xml";
	    mimetypeMap["rf"] = "image/vnd.rn-realflash";
	    mimetypeMap["rgb"] = "image/x-rgb";
	    mimetypeMap["rgs"] = "text/plain";
	    mimetypeMap["rm"] = "application/vnd.rn-realmedia";
	    mimetypeMap["rmi"] = "audio/mid";
	    mimetypeMap["rmp"] = "application/vnd.rn-rn_music_package";
	    mimetypeMap["rmvb"] = "application/vnd.rn-realmedia-vbr";
	    mimetypeMap["roff"] = "application/x-troff";
	    mimetypeMap["rpm"] = "audio/x-pn-realaudio-plugin";
	    mimetypeMap["rqy"] = "text/x-ms-rqy";
	    mimetypeMap["rtf"] = "application/rtf";
	    mimetypeMap["rtx"] = "text/richtext";
	    mimetypeMap["rvt"] = "application/octet-stream";
	    mimetypeMap["ruleset"] = "application/xml";
	    mimetypeMap["s"] = "text/plain";
	    mimetypeMap["safariextz"] = "application/x-safari-safariextz";
	    mimetypeMap["scd"] = "application/x-msschedule";
	    mimetypeMap["scr"] = "text/plain";
	    mimetypeMap["sct"] = "text/scriptlet";
	    mimetypeMap["sd2"] = "audio/x-sd2";
	    mimetypeMap["sdp"] = "application/sdp";
	    mimetypeMap["sea"] = "application/octet-stream";
	    mimetypeMap["searchConnector-ms"] = "application/windows-search-connector+xml";
	    mimetypeMap["setpay"] = "application/set-payment-initiation";
	    mimetypeMap["setreg"] = "application/set-registration-initiation";
	    mimetypeMap["settings"] = "application/xml";
	    mimetypeMap["sgimb"] = "application/x-sgimb";
	    mimetypeMap["sgml"] = "text/sgml";
	    mimetypeMap["sh"] = "application/x-sh";
	    mimetypeMap["shar"] = "application/x-shar";
	    mimetypeMap["shtml"] = "text/html";
	    mimetypeMap["sit"] = "application/x-stuffit";
	    mimetypeMap["sitemap"] = "application/xml";
	    mimetypeMap["skin"] = "application/xml";
	    mimetypeMap["skp"] = "application/x-koan";
	    mimetypeMap["sldm"] = "application/vnd.ms-powerpoint.slide.macroEnabled.12";
	    mimetypeMap["sldx"] = "application/vnd.openxmlformats-officedocument.presentationml.slide";
	    mimetypeMap["slk"] = "application/vnd.ms-excel";
	    mimetypeMap["sln"] = "text/plain";
	    mimetypeMap["slupkg-ms"] = "application/x-ms-license";
	    mimetypeMap["smd"] = "audio/x-smd";
	    mimetypeMap["smi"] = "application/octet-stream";
	    mimetypeMap["smx"] = "audio/x-smd";
	    mimetypeMap["smz"] = "audio/x-smd";
	    mimetypeMap["snd"] = "audio/basic";
	    mimetypeMap["snippet"] = "application/xml";
	    mimetypeMap["snp"] = "application/octet-stream";
	    mimetypeMap["sql"] = "application/sql";
	    mimetypeMap["sol"] = "text/plain";
	    mimetypeMap["sor"] = "text/plain";
	    mimetypeMap["spc"] = "application/x-pkcs7-certificates";
	    mimetypeMap["spl"] = "application/futuresplash";
	    mimetypeMap["spx"] = "audio/ogg";
	    mimetypeMap["src"] = "application/x-wais-source";
	    mimetypeMap["srf"] = "text/plain";
	    mimetypeMap["SSISDeploymentManifest"] = "text/xml";
	    mimetypeMap["ssm"] = "application/streamingmedia";
	    mimetypeMap["sst"] = "application/vnd.ms-pki.certstore";
	    mimetypeMap["stl"] = "application/vnd.ms-pki.stl";
	    mimetypeMap["sv4cpio"] = "application/x-sv4cpio";
	    mimetypeMap["sv4crc"] = "application/x-sv4crc";
	    mimetypeMap["svc"] = "application/xml";
	    mimetypeMap["svg"] = "image/svg+xml";
	    mimetypeMap["swf"] = "application/x-shockwave-flash";
	    mimetypeMap["step"] = "application/step";
	    mimetypeMap["stp"] = "application/step";
	    mimetypeMap["t"] = "application/x-troff";
	    mimetypeMap["tar"] = "application/x-tar";
	    mimetypeMap["tcl"] = "application/x-tcl";
	    mimetypeMap["testrunconfig"] = "application/xml";
	    mimetypeMap["testsettings"] = "application/xml";
	    mimetypeMap["tex"] = "application/x-tex";
	    mimetypeMap["texi"] = "application/x-texinfo";
	    mimetypeMap["texinfo"] = "application/x-texinfo";
	    mimetypeMap["tgz"] = "application/x-compressed";
	    mimetypeMap["thmx"] = "application/vnd.ms-officetheme";
	    mimetypeMap["thn"] = "application/octet-stream";
	    mimetypeMap["tif"] = "image/tiff";
	    mimetypeMap["tiff"] = "image/tiff";
	    mimetypeMap["tlh"] = "text/plain";
	    mimetypeMap["tli"] = "text/plain";
	    mimetypeMap["toc"] = "application/octet-stream";
	    mimetypeMap["tr"] = "application/x-troff";
	    mimetypeMap["trm"] = "application/x-msterminal";
	    mimetypeMap["trx"] = "application/xml";
	    mimetypeMap["ts"] = "video/vnd.dlna.mpeg-tts";
	    mimetypeMap["tsv"] = "text/tab-separated-values";
	    mimetypeMap["ttf"] = "application/font-sfnt";
	    mimetypeMap["tts"] = "video/vnd.dlna.mpeg-tts";
	    mimetypeMap["txt"] = "text/plain";
	    mimetypeMap["u32"] = "application/octet-stream";
	    mimetypeMap["uls"] = "text/iuls";
	    mimetypeMap["user"] = "text/plain";
	    mimetypeMap["ustar"] = "application/x-ustar";
	    mimetypeMap["vb"] = "text/plain";
	    mimetypeMap["vbdproj"] = "text/plain";
	    mimetypeMap["vbk"] = "video/mpeg";
	    mimetypeMap["vbproj"] = "text/plain";
	    mimetypeMap["vbs"] = "text/vbscript";
	    mimetypeMap["vcf"] = "text/x-vcard";
	    mimetypeMap["vcproj"] = "application/xml";
	    mimetypeMap["vcs"] = "text/plain";
	    mimetypeMap["vcxproj"] = "application/xml";
	    mimetypeMap["vddproj"] = "text/plain";
	    mimetypeMap["vdp"] = "text/plain";
	    mimetypeMap["vdproj"] = "text/plain";
	    mimetypeMap["vdx"] = "application/vnd.ms-visio.viewer";
	    mimetypeMap["vml"] = "text/xml";
	    mimetypeMap["vscontent"] = "application/xml";
	    mimetypeMap["vsct"] = "text/xml";
	    mimetypeMap["vsd"] = "application/vnd.visio";
	    mimetypeMap["vsi"] = "application/ms-vsi";
	    mimetypeMap["vsix"] = "application/vsix";
	    mimetypeMap["vsixlangpack"] = "text/xml";
	    mimetypeMap["vsixmanifest"] = "text/xml";
	    mimetypeMap["vsmdi"] = "application/xml";
	    mimetypeMap["vspscc"] = "text/plain";
	    mimetypeMap["vss"] = "application/vnd.visio";
	    mimetypeMap["vsscc"] = "text/plain";
	    mimetypeMap["vssettings"] = "text/xml";
	    mimetypeMap["vssscc"] = "text/plain";
	    mimetypeMap["vst"] = "application/vnd.visio";
	    mimetypeMap["vstemplate"] = "text/xml";
	    mimetypeMap["vsto"] = "application/x-ms-vsto";
	    mimetypeMap["vsw"] = "application/vnd.visio";
	    mimetypeMap["vsx"] = "application/vnd.visio";
	    mimetypeMap["vtt"] = "text/vtt";
	    mimetypeMap["vtx"] = "application/vnd.visio";
	    mimetypeMap["wasm"] = "application/wasm";
	    mimetypeMap["wav"] = "audio/wav";
	    mimetypeMap["wave"] = "audio/wav";
	    mimetypeMap["wax"] = "audio/x-ms-wax";
	    mimetypeMap["wbk"] = "application/msword";
	    mimetypeMap["wbmp"] = "image/vnd.wap.wbmp";
	    mimetypeMap["wcm"] = "application/vnd.ms-works";
	    mimetypeMap["wdb"] = "application/vnd.ms-works";
	    mimetypeMap["wdp"] = "image/vnd.ms-photo";
	    mimetypeMap["webarchive"] = "application/x-safari-webarchive";
	    mimetypeMap["webm"] = "video/webm";
	    mimetypeMap["webp"] = "image/webp";
	    mimetypeMap["webtest"] = "application/xml";
	    mimetypeMap["wiq"] = "application/xml";
	    mimetypeMap["wiz"] = "application/msword";
	    mimetypeMap["wks"] = "application/vnd.ms-works";
	    mimetypeMap["WLMP"] = "application/wlmoviemaker";
	    mimetypeMap["wlpginstall"] = "application/x-wlpg-detect";
	    mimetypeMap["wlpginstall3"] = "application/x-wlpg3-detect";
	    mimetypeMap["wm"] = "video/x-ms-wm";
	    mimetypeMap["wma"] = "audio/x-ms-wma";
	    mimetypeMap["wmd"] = "application/x-ms-wmd";
	    mimetypeMap["wmf"] = "application/x-msmetafile";
	    mimetypeMap["wml"] = "text/vnd.wap.wml";
	    mimetypeMap["wmlc"] = "application/vnd.wap.wmlc";
	    mimetypeMap["wmls"] = "text/vnd.wap.wmlscript";
	    mimetypeMap["wmlsc"] = "application/vnd.wap.wmlscriptc";
	    mimetypeMap["wmp"] = "video/x-ms-wmp";
	    mimetypeMap["wmv"] = "video/x-ms-wmv";
	    mimetypeMap["wmx"] = "video/x-ms-wmx";
	    mimetypeMap["wmz"] = "application/x-ms-wmz";
	    mimetypeMap["woff"] = "application/font-woff";
	    mimetypeMap["woff2"] = "application/font-woff2";
	    mimetypeMap["wpl"] = "application/vnd.ms-wpl";
	    mimetypeMap["wps"] = "application/vnd.ms-works";
	    mimetypeMap["wri"] = "application/x-mswrite";
	    mimetypeMap["wrl"] = "x-world/x-vrml";
	    mimetypeMap["wrz"] = "x-world/x-vrml";
	    mimetypeMap["wsc"] = "text/scriptlet";
	    mimetypeMap["wsdl"] = "text/xml";
	    mimetypeMap["wvx"] = "video/x-ms-wvx";
	    mimetypeMap["x"] = "application/directx";
	    mimetypeMap["xaf"] = "x-world/x-vrml";
	    mimetypeMap["xaml"] = "application/xaml+xml";
	    mimetypeMap["xap"] = "application/x-silverlight-app";
	    mimetypeMap["xbap"] = "application/x-ms-xbap";
	    mimetypeMap["xbm"] = "image/x-xbitmap";
	    mimetypeMap["xdr"] = "text/plain";
	    mimetypeMap["xht"] = "application/xhtml+xml";
	    mimetypeMap["xhtml"] = "application/xhtml+xml";
	    mimetypeMap["xla"] = "application/vnd.ms-excel";
	    mimetypeMap["xlam"] = "application/vnd.ms-excel.addin.macroEnabled.12";
	    mimetypeMap["xlc"] = "application/vnd.ms-excel";
	    mimetypeMap["xld"] = "application/vnd.ms-excel";
	    mimetypeMap["xlk"] = "application/vnd.ms-excel";
	    mimetypeMap["xll"] = "application/vnd.ms-excel";
	    mimetypeMap["xlm"] = "application/vnd.ms-excel";
	    mimetypeMap["xls"] = "application/vnd.ms-excel";
	    mimetypeMap["xlsb"] = "application/vnd.ms-excel.sheet.binary.macroEnabled.12";
	    mimetypeMap["xlsm"] = "application/vnd.ms-excel.sheet.macroEnabled.12";
	    mimetypeMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	    mimetypeMap["xlt"] = "application/vnd.ms-excel";
	    mimetypeMap["xltm"] = "application/vnd.ms-excel.template.macroEnabled.12";
	    mimetypeMap["xltx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.template";
	    mimetypeMap["xlw"] = "application/vnd.ms-excel";
	    mimetypeMap["xml"] = "text/xml";
	    mimetypeMap["xmp"] = "application/octet-stream";
	    mimetypeMap["xmta"] = "application/xml";
	    mimetypeMap["xof"] = "x-world/x-vrml";
	    mimetypeMap["XOML"] = "text/plain";
	    mimetypeMap["xpm"] = "image/x-xpixmap";
	    mimetypeMap["xps"] = "application/vnd.ms-xpsdocument";
	    mimetypeMap["xrm-ms"] = "text/xml";
	    mimetypeMap["xsc"] = "application/xml";
	    mimetypeMap["xsd"] = "text/xml";
	    mimetypeMap["xsf"] = "text/xml";
	    mimetypeMap["xsl"] = "text/xml";
	    mimetypeMap["xslt"] = "text/xml";
	    mimetypeMap["xsn"] = "application/octet-stream";
	    mimetypeMap["xss"] = "application/xml";
	    mimetypeMap["xspf"] = "application/xspf+xml";
	    mimetypeMap["xtp"] = "application/octet-stream";
	    mimetypeMap["xwd"] = "image/x-xwindowdump";
	    mimetypeMap["z"] = "application/x-compress";
	    mimetypeMap["zip"] = "application/zip";
	}
