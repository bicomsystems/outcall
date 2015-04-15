## What is OutCALL? ##

This page was originally written about the OutCALL application. But as [Bicom Systems](http://www.bicomsystems.com) has advanced and taken steps forward, OutCALL has been replaced by [gloCOM](http://marketing.bicomsystems.com/files/marketing/gloCOM.pdf).
For details, watch the following video http://www.youtube.com/watch?v=naL7EuwNKaw, or check out  [gloCOM](http://marketing.bicomsystems.com/files/marketing/gloCOM.pdf) features.

[OutCALL](http://www.bicomsystems.com/products/outcall) is an
application designed for integration with Microsoft Outlook while placing
and receiving phone calls. The application includes: Outlook contacts
integration, incoming calls pop-up window notification, click to dial from
email/contact.

The [Enterprise Edition](http://www.bicomsystems.com/products/outcall-enterprise) allows integration with Microsoft Exchange.

After over 1000 downloads as a free application, [Bicom Systems](http://www.bicomsystems.com) has decided to offer [OutCALL](http://www.bicomsystems.com/products/outcall) in open
source format in order to further stimulate development of Asterisk and
related open source projects.

[OutCALL](http://www.bicomsystems.com/products/outcall) is licensed
under BSD Open Source license.

[OutCALL](http://www.bicomsystems.com/products/outcall) works with
the [Business PBX](http://www.bicomsystems.com/products/business-pbx),
[Call Center PBX](http://www.bicomsystems.com/products/call-center-pbx), and [Multi-Tenant PBX](http://www.bicomsystems.com/products/multi-tenant-pbx) (a [Multi-Tenant](http://www.bicomsystems.com/products/multi-tenant-pbx) IP PBX solution) editions of [PBXware](http://www.bicomsystems.com/products/pbxware) as well as with [gloCOM](http://www.bicomsystems.com/products/glocom) and [Softswitch](http://www.bicomsystems.com/products/multi-tenant-pbx).

For more information, see the [Bicom Systems](http://www.bicomsystems.com) website or browse through our [Products](http://www.bicomsystems.com/products).

After over 12,000 downloads on Source Forge, OutCALL has been moved to Google Code.

**Note: [Bicom Systems](http://www.bicomsystems.com) only gives support for [OutCALL 2.0](http://www.bicomsystems.com/products/outcall) and above - as such it does not work with vanilla Asterisk.**

## OutCALL features are: ##

  * Integration with one or unlimited system extensions (SIP/IAX)
  * Automatic integration with Microsoft Outlook 2000 and higher
  * Call History
  * Real time call notifications via pop windows
  * Placing calls within Microsoft Outlook, email message or contact
  * Automatic contacts data update
  * Automatic application updates notifications
  * Clear debug information
  * Full Asterisk integration

![http://outcall.googlecode.com/files/OutCALL.png](http://outcall.googlecode.com/files/OutCALL.png)


# Changelog: #

### Version 1.60 (2010-10-05) ###

- Added following languages: German, Italian, Russian

- Fixed issues with MS Exchange GAL

- Fixed crashing of Outlook 2007 on Windows 7 and Windows Vista

- Fixed working directory for the OutCALL shortcut in Start Menu.

- When a new contact is added into Outlook from OutCALL, OutCALL's database is automatically synced with Outlook.

- Fixed adding of new contacts to Outlook 2007 when Outlook is not running.

- Fixed issue with focus for the phone number text field (Dial window).

- Fixed dialing a contact with only "Company" name set.

- Fixed dialing issue. First time it would place the call, second time it would fail.

- Fixed issue with removing of items in Call History.

- Fixed issue when dialing out from Call History.

- Fixed issue with Call History for placed calls. The number which was called was incorrect in the history of calls.

- Fixed critical bug in MAPI interface (Outlook 2002). Invalid flags were passed to IMsgStore::OpenEntry.

- Added missing Outlook dependencies required for compiling.

- Project is updated to Visual Studio 2010. There are also project files for VS 2005.


### Version 1.55 (2008-07-02) ###

- Added support for Exchange Server.


### Version 1.50 (2008-06-13) ###

- Added support for UNICODE. It is possible to compile OutCALL in Visual Studio with or without UNICODE support with the same codebase.

- OutCALL now uses Extended MAPI to load contacts from Outlook. It is now much faster and reliable.

- Added Outlook Import Rules (Settings->Outlook Import Rules). Theese rules apply for phone numbers loaded from Outlook as well as for outgoing calls placed from OutCALL.

- Added support for calls coming from Queue, showing the Queue number and CallerID.

- It is now possible to call from Outlook even if the selected contact is not in OutCALL's database.

- Added Bosnian, Spanish and Slovak translation.

- Added logging useful for debugging purposes ($APPLICATION\_DATA\OutCALL\outcall.log).

- Added option to cancel process of loading Outlook Contacts.

- When invalid values are specified in Settings dialog for Server/Username/Port, user is automatically moved to the Server tab and the appropriate field is focused.

- Added "GNU gettext" dependencies to the Source Code package.


**BUG fixes:**

- Fixed critical bug when user is logged on as a domain user.

- Fixed memory leaks in MAPIEx interface.

- Fixed bug with memory leak in Call dialog.

- Fixed bug with Outlook plugin integration on a fresh Windows install.

- Fixed bug with Dial Window focus.

- Fixed problems with Outlook integration when running OutCALL as a normal user (both Windows XP and Vista).

- Some strings in user interface could not be translated. Fixed.

- "Incoming strip digits" option is fixed so that the largest prefix will be matched first.

- Fixed bug when Importing Contacts from CSV file.

- Added C/C++ runtime dependencies into setup.

- OutCALL stripes out "-" from the phone number before dialing out.

- In "auth" version of OutCALL it was possible to place calls from extensions for which user was not authorized in OutCALL. Fixed.

- Fixed GUI control alignment in Settings dialog.

- Reorganized some internal staff in the code.

