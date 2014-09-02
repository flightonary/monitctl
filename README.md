monitctl
========
Monit [http://mmonit.com/monit/] is very useful monitoring tool. However monit optional action requires supervisor authority (e.g. sudo) in the case that monit is running as root. It is also same that monit executable file has been given the setuid attribute because it checks real uid in execution.

Monitctl executes monit as root when monitctl has been given the setuid attribute (of course monitctl owner is root). Additionally executable monit optional action arguments is limitable via monitctl configuration file.

Usage
-----
1. run ```$ rake```
2. copy 'monitctl' onto your executable path.
4. copy 'monitctl.conf' to /etc/monitctl.conf.
5. run ```$ monitctl```

If you want to specify monitctl config file path, you may run ```$ rake config_file=/path/to/file```.  
(default is /etc/monitctl.conf)

How it works
------------
    $ ls -l /usr/bin/monitctl
    $ -rwsr-xr-x 1 root root 9.6K  9月  2 23:58 /usr/bin/monitctl*
    $ monitctl start nginx

Contributors
------------
jetbeaver (jetbeaver@gmail.com)

History
-------
03/09/2014  v1.0
