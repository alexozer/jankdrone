#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Some commonly-used functions for pyratemp.

:Version:   0.3.2
:Requires:  Python >=2.6 / 3.x

:Author:    Roland Koebler (rk at simple-is-better dot org)
:Copyright: Roland Koebler
:License:   MIT/X11-like, see __license__

:RCS:       $Id: tools.py,v 1.7 2013/09/17 07:45:04 rk Exp $
"""
from __future__ import unicode_literals
import os, time, datetime, codecs, socket, email.utils
import pyratemp

__version__ = "0.3.2"
__author__  = "Roland Koebler <rk at simple-is-better dot org>"
__license__ = pyratemp.__license__

#-----------------------------------------
# html-creation

def html(template, data, xmlreplace=True, filename=None, chmod=None):
    """Create a HTML-page from a pyratemp-template.

    Additionally defined variables (may be overwritten by data):

    - date, mtime_CCYYMMDD: current date as YYYY-MM-DD

    :Parameters:
        - template:   template-file
        - data:       data for the template (dictionary)
        - xmlreplace: if True, encode result in ascii with xmlcharrefreplace
        - filename:   write result to this file (in utf-8)
        - chmod:      access permissions for the written file
    :Returns:
        the filled template

    :Note: If filename is given, the result is first written to
           filename.tmp, then the permissions are set, and then
           the file is renamed.
    """
    timestamp = time.strftime("%Y-%m-%d")
    t = pyratemp.Template(filename=template, data={
        "date"           : timestamp,
        "mtime_CCYYMMDD" : timestamp
        })
    result = t(**data)

    if xmlreplace:
        result = result.encode("ascii", 'xmlcharrefreplace')

    if filename:
        if xmlreplace:
            f = open(filename + ".tmp", "wb")
        else:
            f = codecs.open(filename + ".tmp", "w", encoding="utf-8")
        f.write(result)
        f.close()
        if chmod:
            os.chmod(filename+".tmp", chmod)
        os.rename(filename+".tmp", filename)

    return result

#----------------------
# mail-creation

def mail(maildir, template, data, messageid_domainname=None):
    """Create a mail from a pyratemp-template and store it in a maildir.

    :Parameters:
        - maildir:  maildir-directory
        - template: template-file
        - data:     data for the template (dictionary)
        - messageid_domainname: domainname for the created messageid
    :Returns:
        the filename (without path) of the created mail

    :Notes:
        - "messageid" and "date" are are added to ``data`` if they are
           not yet there.
        - For convenience, the variables in data can both be accessed
          directly and by using data[...].
        - The mail is first written to maildir/tmp/ and then renamed
          to maildir/new/.
        - The created file is encoded in utf-8.
    """
    # create mail
    t = time.time()
    if "date" not in data:
        data["date"] = email.utils.formatdate(t, localtime=True)
    if "messageid" not in data:
        if messageid_domainname is None:
            messageid_domainname = socket.gethostname()
        data["messageid"] = datetime.datetime.fromtimestamp(t).strftime("%Y%m%d%H%M%S.%f") + "@" + messageid_domainname
    t = pyratemp.Template(filename=template, escape=pyratemp.MAIL_HEADER)
    result = t(data=data, **data)

    # write mail to maildir
    mailname = "%fP%d.%s" % (time.time(), os.getpid(), socket.gethostname().replace("/", r"\057").replace(":", r"\072"))
    mailtmp = os.path.join(maildir, "tmp", mailname)
    mailnew = os.path.join(maildir, "new", mailname)
    f = codecs.open(mailtmp, "w", encoding="utf-8")
    f.write(result)
    f.close()
    os.rename(mailtmp, mailnew)

    return mailname

#-----------------------------------------

