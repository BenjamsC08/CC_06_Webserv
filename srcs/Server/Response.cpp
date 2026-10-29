/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:13 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:51:11 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response():_contentType(""), _headerBuffer(""), _bodyBuffer(""), _path(""), _response(""),
    _bytesSent(0), _totalByte(0), _statusCode(0), _fdToWrite(0), _bodyTooBig(false)
{}

Response::Response(const Response & src):_contentType(src._contentType), _headerBuffer(src._headerBuffer),
    _bodyBuffer(src._bodyBuffer), _path(src._path), _response(src._response), _bytesSent(src._bytesSent), 
    _totalByte(src._totalByte), _statusCode(src._statusCode), _fdToWrite(src._fdToWrite),
    _bodyTooBig(src._bodyTooBig), _errorPath(src._errorPath)
{}

Response & Response::operator=(const Response & src)
{
    if (this != &src)
    {
        _contentType = src._contentType;
        _headerBuffer = src._headerBuffer;
        _bodyBuffer = src._bodyBuffer;
        _path = src._path;
        _response = src._response;
        _bytesSent = src._bytesSent;
        _totalByte = src._totalByte;
        _statusCode = src._statusCode;
        _fdToWrite = src._fdToWrite;
        _bodyTooBig = src._bodyTooBig;
        _errorPath = src._errorPath;
    }
    return (*this);
}

Response::~Response()
{}

static std::string content_type_finder(std::string filename)
{
    if (filename.find(".html") != std::string::npos)
        return ("text/html");
    else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
        return ("image/jpg");
    else if (filename.find(".png") != std::string::npos)
        return ("image/png");
    else if (filename.find(".mp4") != std::string::npos)
        return ("video/mp4");
    else if (filename.find(".mp3") != std::string::npos)
        return ("audio/mp3");
    else if (filename.find(".ico") != std::string::npos)
        return ("image/x-icon");
    else if (filename.find(".css") != std::string::npos)
        return ("text/css");
    else
        return ("");
}

std::string Response::themeInjector(Request & request, std::string body)
{
    const std::map<std::string, std::string> & head = request.getHeader();
    std::map<std::string, std::string>::const_iterator it = head.find("cookie");
    size_t start = body.find("{{THEME_FILENAME}}");
    if (start != std::string::npos)
    {
        std::string txt = "{{THEME_FILENAME}}";
        if (it != head.end() && it->second.find("theme=light") != std::string::npos)
            body.replace(start, txt.length(), "light.css");
        else
            body.replace(start, txt.length(), "dark.css");
    }

    return (body);
}
void        Response::errorConstructors(std::string errorCode, Request & request)
{
    std::map<std::string, std::string> ErrorPhrase;
    ErrorPhrase["400"] = "Bad Request";
    ErrorPhrase["401"] = "Unauthorized";
    ErrorPhrase["403"] = "Forbidden";
    ErrorPhrase["404"] = "Not Found";
    ErrorPhrase["405"] = "Method Not Allowed";
    ErrorPhrase["408"] = "Request Timeout";
    ErrorPhrase["411"] = "Length Required";
    ErrorPhrase["413"] = "Content Too Large";
    ErrorPhrase["415"] = "Unsupported Media Type";
    ErrorPhrase["500"] = "Internal Server Error";
    ErrorPhrase["502"] = "Bad Gateway";
    ErrorPhrase["504"] = "Gateway Timeout";

    std::string body;
    std::string mimeType = "text/html";
    char        *endptr;
    int         codeInt = static_cast<int>(strtod(errorCode.c_str(), &endptr));

    std::map<int, std::string>::iterator it = _errorPath.find(codeInt);

    if (errorCode != "500" && it != _errorPath.end() && !it->second.empty())
    {
        std::string file = it->second;

        struct stat check;
        if (stat(file.c_str(), &check) == 0)
        {
            std::ifstream input(file.c_str(), std::ios::binary);
            if (input.is_open())
            {
                std::string line;
                while(getline(input,line))
                {
                    body.append(line);
                }
            }
            else
            {
                DEBUG("Error constructor's open failed, Error 500\n");
                return (errorConstructors("500",request));
            }
        }
    }

    if (body.empty())
    {
        body.append("<!DOCTYPE html>");
        body.append("<html><head><meta charset='UTF-8'>");
        body.append("<title>" + errorCode + " " + ErrorPhrase[errorCode] + "</title>");
        body.append("<link rel='stylesheet' href='/css/{{THEME_FILENAME}}'>");
        body.append("</style></head><body>");

        body.append("<div class='window'>");
        body.append("<div class='title-bar'><span>" + errorCode + ".EXE</span><span>X</span></div>");

        body.append("<div class='content'>");
        body.append("<h3>" + errorCode + " - " + ErrorPhrase[errorCode] + "</h3>");
        body.append("Something went wrong.<br><br>");
        body.append("<a href='/'>Go Home</a>");
        body.append("</div>");

        body.append("</div>");
        body.append("</body></html>");
    }

    body = themeInjector(request, body);

    std::ostringstream oss;
    oss << "HTTP/1.1 " << errorCode << " " << ErrorPhrase[errorCode] << "\r\n";
    oss << "Connection: " << _connection << "\r\n";
    oss << "Content-Type: " << mimeType << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n\r\n";
    oss << body;
    _response = oss.str();


}

void    Response::galleryHandle(Request & request)
{
    struct dirent *ent;
    DIR *dir;
    size_t count = 0;
    size_t len = 0;

    std::string file = "./assets/pages/gallery.html";
    
    struct stat check;
    if (stat(file.c_str(), &check) == 0)
    {
        std::ifstream input(file.c_str(), std::ios::binary);
        if (!(check.st_mode & S_IWUSR) || !(check.st_mode & S_IRUSR))
        {
            DEBUG("Gallery read or write permission denied, error 403\n");
            errorConstructors("403",request);
            return;
        }
        else if (input.fail())
        {
            DEBUG("Gallery open has failed, error 500\n");
            errorConstructors("500",request);
            return;
        }
        else
        {

            std::string line;
            std::string body;
            if (_response.empty())
            {
                _response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
                _response += "Connection: " + _connection + "\r\n";
                _response += "Content-Length: ";
            }

            while(getline(input, line))
            {
                body.append(line);
            }


            if ((dir = opendir("./assets/upload")) != NULL)
            {
                std::string images;

                images.append("\t<div class='content'><div class='gallery-grid'>\n\t\t");

                while ((ent = readdir(dir)) != NULL)
                {
                    if (ent->d_name[0] == '.')
                        continue;
                    count++;
                    images.append("<div class='image-card'>\n\t\t\t<img src='/upload/");
                    images.append(ent->d_name);
                    images.append("' alt='Uploaded Image'>\n\t\t\t<div class='info'>\n\t\t\t\t<span>");
                    images.append(ent->d_name);
                    images.append("</span>\n\t\t\t\t<button class='win-button delete-btn' onclick=\"deleteImage('/upload/");
                    images.append(ent->d_name);
                    images.append("')\">DELETE</button>\n\t\t\t</div>\n\t\t</div>\n\n");
                }

                if (count == 0)
                {
                    images.append("<div class='empty-message'>");
                    images.append("<h2 style='color: #808080;'>C:\\RAT_SERVICES\\UPLOADS is empty.</h2>");
                    images.append("<p>Please upload rat data to populate the gallery.</p></div>");
                }

                images.append("</div></div>");

                images.append("<script>");
                images.append("async function deleteImage(imagePath) {");
                images.append("  if (!confirm('Are you sure?')) return;");
                images.append("  const res = await fetch(imagePath, { method: 'DELETE' });");
                images.append("  if (res.ok) {");
                images.append("     location.reload();");
                images.append("  } else {");
                images.append("     const errorHtml = await res.text();");
                images.append("     document.open();");
                images.append("     document.write(errorHtml);");
                images.append("     document.close();");
                images.append("     }");
                images.append("}");
                images.append("</script></body></html>");




                size_t start = body.find("{{THEME_FILENAME}}");
                if (start != std::string::npos)
                    body = themeInjector(request, body);

                
                len = body.length();
                len += images.length();
                std::stringstream ss;
                ss << len;
                _response.append( ss.str() + "\r\n\r\n" + body + images);
                closedir(dir);
            }
            else
            {
                DEBUG(" Couldn't open /assets/upload, error 403\n");
                errorConstructors("403",request);
                return;
            }
        }
    }
    else
    {
        DEBUG(" Gallery html not found, error 404\n");
        errorConstructors("404",request);
        return;
    }
}

std::string Response::AutoIndexCreate(std::string directoryPath, std::string requestPath, Request & request)
{
    std::string html = "<html><head><link rel='stylesheet' href='/css/{{THEME_FILENAME}}'>";
    html += "</head><body>";
    html += "<div class='window'><div class='title-bar'>Exploring - " + requestPath + "</div>";
    html += "<div class='file-list'><table><thead><tr><th>Name</th><th>Size</th><th>Type</th></tr></thead><tbody>";

    struct dirent *ent;
    DIR *dir = opendir(directoryPath.c_str());
    size_t count = 0;

    if (dir) {
        while ((ent = readdir(dir)) != NULL)
        {
            std::string name = ent->d_name;
            if (name[0] == '.')
                continue;
            count++;
            html.append("<tr>");
            html.append("<td><a href='" + name + "'>" + name + "</a></td>");
            html.append("<td>--</td>");
            html.append("<td>" + std::string(ent->d_type == DT_DIR ? "Directory" : "File") + "</td>");
            html.append("</tr>");
        }
        closedir(dir);
    }

    if (count == 0)
    {
        html.append("<tr><td colspan='3' class='empty-cell'>");
        html.append("<h2 class='empty-title'>C:\\RAT_SERVICES\\" + requestPath + " is empty.</h2>");
        html.append("<p class='empty-text'>No files found in this directory.</p>");
        html.append("</td></tr>");
    }

    html.append("</tbody></table></div></div></body></html>");
    
    html = themeInjector(request, html);

    return (html);
}

void    Response::handleCookieTheme(Request & request)
{
    std::string theme = "light";
    if (request.getPath().find("theme=dark") != std::string::npos)
        theme = "dark";
    
    _response = "HTTP/1.1 303 See Other\r\n";
    _response.append("Location: /\r\n");
    _response.append("Set-Cookie: theme=" + theme + "; Path=/; Max-Age=31536000\r\n");
    _response.append("Content-Length: 0\r\n\r\n");
}

void Response::prepareResponse(Request & request)
{
    if (request.getPath().find("/set-theme") == 0)
    {
        handleCookieTheme(request);
        return;
    }

    struct stat check;

    std::string root = request.getHttpRule().getRootPath();
    std::string file = root + request.getPathNoPrefix();
    if (!file.empty() && file[0] == '/')
        file = "." + file;

    if (stat(file.c_str(), &check) == 0)
    {
        if (S_ISDIR(check.st_mode))
        {
            if (!request.getPath().empty() && request.getPath()[request.getPath().size() - 1] != '/')
            {
                _response = "HTTP/1.1 301 Moved Permanently\r\n";
                _response.append("Location: " + request.getPath() + "/\r\n");
                _response.append("Content-Length: 0\r\n\r\n");
                return;
            }
            if (request.getToRedir() && !request.getRedir().empty())
            {
                _response = "HTTP/1.1 303 See Other\r\n";
                _response.append("Location: " + request.getRedir() + "\r\n");
                _response.append("Content-Length: 0\r\n\r\n");
                return;
            }
            if (request.getMethod() == "POST")
            {
                (void)request.getMethod();
            }
            else if (!request.getHttpRule().getPathNoListing().empty())
            {
                file = "." + root + request.getPath() + request.getHttpRule().getPathNoListing();
            }
            else if (request.getHttpRule().isListingAct())
            {
                std::string autoIndexBody = AutoIndexCreate(file, request.getPathNoPrefix(), request); 
                _response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
                _response += "Connection: " + _connection + "\r\n";
                _response += "Content-Length: ";
                std::stringstream ss;
                ss << autoIndexBody.length();
                _response += ss.str() + "\r\n\r\n" + autoIndexBody;
                return;
            }   
            else
            {
                DEBUG("Path is a folder but auto index is disabled, and there is no default in the location, Error 403\n");
                errorConstructors("403",request);
                return;
            }
        }
    }
    else if (request.getMethod() != "POST")
    {    
        DEBUG("This path doesn't lead anywhere, error 404\n");
        errorConstructors("404",request);
        return;
    }
    
    if (!request.getErrorCode().empty())
    {
        DEBUG("We have found an error in parsing and it's " << request.getErrorCode() << "\n");
        errorConstructors(request.getErrorCode(), request);
        return;
    }
    if (request.getMethod() == "GET" && request.getHttpRule().getMethods().flag.GET_METHOD == 1)
    {
        if (request.getPath() == "/gallery.html")
        {
            galleryHandle(request);
            return;
        }
        std::ifstream input(file.c_str(), std::ios::binary);
        if (input.fail())
        {
            DEBUG("HTML file not found, error 404\n");
            errorConstructors("404",request);
            return;
        }
        else
        {
            std::string line;
            std::string body;
            if (content_type_finder(file) == "")
            {
                DEBUG("We don't handle this media type, error 415\n");
                errorConstructors("415",request);
                return;
            }
            else
            {
                _response = "HTTP/1.1 200 OK\r\nContent-Type: ";
                _response.append(content_type_finder(file));
                _response.append("\r\n");
                _response.append("Connection: " + _connection + "\r\n");
                _response.append("Content-Length: ");
            
                if (content_type_finder(file) == "image/png" || content_type_finder(file) == "image/jpg" || content_type_finder(file) == "image/x-icon")
                {
                    
                    input.seekg(0, input.end);
                    int len = input.tellg();
                    input.seekg(0, input.beg);
                    std::vector<char> body(len);
                    input.read(body.data(), len);
                    input.close();
                    std::string lenStr;
                    std::stringstream ss;
                    ss << len;
                    ss>>lenStr;
                    _response.append(lenStr);
                    _response.append("\r\n\r\n");
                    _response.append(body.data(), len);
                }
                else
                {
                    while(getline(input, line))
                        body.append(line + "\n");
                
                    size_t start = body.find("{{THEME_FILENAME}}");
                    if (start != std::string::npos)
                        body = themeInjector(request, body);

                    int len = body.length();
                    std::string lenStr;
                    std::stringstream ss;
                    ss << len;
                    ss >> lenStr;
                    _response.append(lenStr);
                    _response.append("\r\n\r\n");
                    _response.append(body);
                }
            }
        }
    }
    else if (request.getMethod() == "DELETE" && request.getHttpRule().getMethods().flag.DELETE_METHOD == 1)
    {
        if (request.getPath().find("..") != std::string::npos)
        {
            DEBUG("Trying to path traversal, error 400\n");
            errorConstructors("400",request);
            return;
        }
        std::string dirPath = file.substr(0, file.find_last_of("/\\"));

        struct stat fileCheck;
        struct stat dirCheck;
        if (stat(file.c_str(), &fileCheck) == -1)
        {
            DEBUG("Trying to delete a file that doesnt exist, error 404\n");
            errorConstructors("404",request);
            return;
        }

        if(S_ISDIR(fileCheck.st_mode))
        {
            DEBUG("Cannot delete a folder, error 403\n");
            errorConstructors("403",request);
            return;
        }

        if (stat(dirPath.c_str(), &dirCheck) == -1)
        {
            DEBUG("Server fail, this directory seems to not exist but previous file check didn't trigger, error 500\n");
            errorConstructors("500",request);
            return; 
        }

        if (!(dirCheck.st_mode & S_IWUSR) || !(dirCheck.st_mode & S_IXUSR) || !(dirCheck.st_mode & S_IRUSR))
        {
            DEBUG("No permission to delete in the folder, error 403\n");
            errorConstructors("403",request);
            return;
        }
    

        if (unlink(file.c_str()) == 0)
        {
            _response = "HTTP/1.1 204 No Content\r\n";
            _response += "Connection: " + _connection + "\r\n\r\n";
        }
        else
        {
            DEBUG("File and Permissions are ok but unlink fail, error 500\n");
            errorConstructors("500",request);
            return;
        }


    }
    else if (request.getMethod() == "POST" && (request.getHttpRule().getMethods().flag.POST_METHOD == 1 /*&& request.getHttpRule().isUploadAct()*/))
    {
        if (_bodyTooBig == true)
        {
            DEBUG("Response: body too big, error 413\n");
            errorConstructors("413",request);
            return;
        }
        else if (request.getSizeNotFound())
        {
            DEBUG("Response: Size not found, error 411\n");
            errorConstructors("411",request);
            return;
        }
        else
        {
            if (!request.getBody().empty())
            {
                std::map<std::string, std::string>::const_iterator it = request.getHeader().find("content-type");

                if (it != request.getHeader().end() && it->second.find("multipart/form-data;") != std::string::npos)
                {
                    multipartHandle(request);
                }
                else if (it != request.getHeader().end() && it->second.find("application/x-www-form-urlencoded;") != std::string::npos)
                {
                    return;
                }
                else
                {
                    uploadHandle(request);
                }
            }

            std::string statusCode;
            std::string statusPhrase;

            if (request.getCreated())
            {
                statusCode = "201";
                statusPhrase = "Created";
            }
            else
            {
                statusCode = "200";
                statusPhrase = "OK";
            }
    
            std::string html;

            html.append("<!DOCTYPE html>");
            html.append("<html><head><meta charset='UTF-8'>");
            html.append("<title>" + statusCode + " " + statusPhrase + "</title>");
            html.append("<link rel='stylesheet' href='/css/{{THEME_FILENAME}}'>");
            html.append("</style></head><body>");
        
            html.append("<div class='window'>");
            html.append("<div class='title-bar'><span>" + statusCode + ".EXE</span><span>X</span></div>");
        
            html.append("<div class='content'>");
            html.append("<h3>" + statusCode + " - " + statusPhrase + "</h3>");
            html.append("Upload succesful.<br><br>");
            html.append("<a href='/gallery.html'>Go Check it out!</a>");
            html.append("</div>");
        
            html.append("</div>");
            html.append("</body></html>");

            html = themeInjector(request, html);
        
            std::ostringstream oss;
            oss << "HTTP/1.1 " << statusCode << " " << statusPhrase << "\r\n";
            oss << "Connection: " << _connection << "\r\n";
            oss << "Content-Type: text/html\r\n";
            oss << "Content-Length: " << html.size() << "\r\n\r\n";
            oss << html;
            _response = oss.str();

        }
    }
    else
    {
        DEBUG("Response: method not used, error 405\n");
        errorConstructors("405",request);
        return;
    }
}

void        Response::uploadHandle(Request & request)
{
    std::string filename;
    if (request.getHeader().find("content-disposition") != request.getHeader().end())
    {
        std::map<std::string, std::string>::const_iterator it = request.getHeader().find("content-disposition");
        size_t start = 0;
        size_t end = 0;
        start = it->second.find("\"");
        end = it->second.find("\"", start);
        filename = it->second.substr(start, end - start);
    }

    if (request.getPathNoPrefix()[request.getPathNoPrefix().size() - 1] == '/')
    {
        DEBUG("Trying to upload without a filename. Error 400\n");
        errorConstructors("400", request);
    }
    else
    {
        filename = request.getPathNoPrefix();
        std::string fullname = request.getHttpRule().getUploadPath() + "/" + filename.c_str();
        
        struct stat check;
        if (stat(fullname.c_str(), &check) == -1)
            request.setCreated(true);
        
        std::ofstream toUpload((request.getHttpRule().getUploadPath() + "/" + filename).c_str(), std::ios::binary);
        if (!toUpload.is_open())
        {
            DEBUG("Open failed. Error 500\n");
            errorConstructors("500", request);
            return;
        }
        toUpload.write(request.getBody().c_str(), request.getBody().size());
        toUpload.close();
    }
}

void        Response::multipartHandle(Request & request)
{
    if (request.getContentLength() > request.getBody().size())
    {
        DEBUG("The size doesn't match content length, error 400\n");
        errorConstructors("400", request);
        return;
    }
    std::map<std::string, std::string>::const_iterator it = request.getHeader().find("content-type");
    std::string boundary = "--" + it->second.substr(it->second.find("boundary=") + 9);

    size_t start = request.getBody().find(boundary);
    if (start == std::string::npos)
    {
        DEBUG("There is no boundary, error 400\n");
        errorConstructors("400", request);
        return;
    }
    size_t nameStart = request.getBody().find("filename=\"", start);
    if (nameStart == std::string::npos)
    {
        DEBUG("There is no boundary, error 400\n");
        errorConstructors("400", request);
        return;
    }

    size_t nameEnd = request.getBody().find("\"", nameStart + 10);
    std::string filename = request.getBody().substr(nameStart + 10, nameEnd - (nameStart + 10));
    std::ofstream toUpload((request.getHttpRule().getUploadPath() + "/" + filename).c_str(), std::ios::binary);
    if (!toUpload.is_open())
    {
        DEBUG("Open failed. Error 500\n");
        errorConstructors("500", request);
        return;
    }

    size_t pos = request.getBody().find(boundary);
    size_t next_pos = request.getBody().find("\r\n\r\n", pos);
    pos = next_pos + 4;

    next_pos = request.getBody().find(boundary, pos);
    if (next_pos > 2)
    {
        std::string imgData = request.getBody().substr(pos, (next_pos - 2) - pos);
        toUpload.write(imgData.c_str(), imgData.size());
        toUpload.close();
    }
    else
    {
        DEBUG("Open failed. Error 500\n");
        errorConstructors("500", request);
        return;
    }
}



























std::string Response::getHeaderBuffer()
{
    return(_headerBuffer);
}
std::string Response::getBodyBuffer()
{
    return(_bodyBuffer);
}

std::string Response::getPath()
{
    return(_path);
}

std::string Response::getResponse()
{
    return(_response);
}

int      Response::getByteSent()
{
    return(_bytesSent);
}

int      Response::getTotalByte()
{
    return(_totalByte);
}
int         Response::getStatusCode()
{
    return(_statusCode);
}
int         Response::getFdToWrite()
{
    return(_fdToWrite);
}

std::string Response::getContentType()
{
    return(_contentType);
}

void        Response::setBytesent(int toSet)
{
    _bytesSent = toSet;

}

void        Response::setTotalBytes(int toSet)
{
    _totalByte = toSet;
}

void        Response::setResponse(std::string toSet)
{
    _response = toSet;
}

void        Response::setConnection(std::string connection)
{
    _connection = connection;
}
void        Response::setErrorPath(std::map<int, std::string> ErrorPath)
{
    _errorPath = ErrorPath;
}

void        Response::reset()
{
    _contentType.clear();
     _headerBuffer.clear();
     _bodyBuffer.clear();
     _path.clear();
     _response.clear();
     _bytesSent = 0;
     _totalByte = 0;
     _statusCode = 0;
     _fdToWrite = -1;
     _bodyTooBig = false;
     _connection.clear();
     _errorPath.clear();
}
