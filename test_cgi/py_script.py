
import cgi, cgitb              
  
form = cgi.FieldStorage()      
your_name = form.getvalue('name')    

company = form.getvalue('company')   
  
print ("Content-type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>First CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello, {} is working in {} </h2>".format(your_name, company))
# while 1:
# 	continue

print ("</body>")
print ("</html>")