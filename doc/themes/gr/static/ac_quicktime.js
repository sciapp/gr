/*

File: AC_QuickTime.js

Abstract: This file contains functions to generate OBJECT and EMBED tags for QuickTime content.

Version: <1.2.1>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006-2007 Apple Computer, Inc., All Rights Reserved

*/ 

/*
 * This file contains functions to generate OBJECT and EMBED tags for QuickTime content. 
 */

/************** LOCALIZABLE GLOBAL VARIABLES ****************/

var gArgCountErr =	'The "%%" function requires an even number of arguments.'
				+	'\nArguments should be in the form "atttributeName", "attributeValue", ...';

/******************** END LOCALIZABLE **********************/

var gTagAttrs				= null;
var gQTGeneratorVersion		= 1.2;
var gQTBehaviorID			= "qt_event_source";
var gQTEventsEnabled		= false;

function AC_QuickTimeVersion()	{ return gQTGeneratorVersion; }

function _QTComplain(callingFcnName, errMsg)
{
    errMsg = errMsg.replace("%%", callingFcnName);
	alert(errMsg);
}

function _QTIsMSIE()
{
    var ua = navigator.userAgent.toLowerCase();
	var msie = /msie/.test(ua) && !/opera/.test(ua);

	return msie;
}


function _QTGenerateBehavior()
{
	return objTag = '<!--[if IE]>'
				 + '<object id="' + gQTBehaviorID + '" classid="clsid:CB927D12-4FF7-4a9e-A169-56E4B8A75598"></object>'
				 + '<![endif]-->';
}

function _QTPageHasBehaviorObject(callingFcnName, args)
{
	var haveBehavior = false;
	var objects = document.getElementsByTagName('object');
	
	for ( var ndx = 0, obj; obj = objects[ndx]; ndx++ )
	{
		if ( obj.getAttribute('classid') == "clsid:CB927D12-4FF7-4a9e-A169-56E4B8A75598" )
		{
			if ( obj.getAttribute('id') == gQTBehaviorID )
				haveBehavior = false;
			break;
		}
	}

	return haveBehavior;
}


function _QTShouldInsertBehavior()
{
	var		shouldDo = false;

	if ( gQTEventsEnabled && _QTIsMSIE() && !_QTPageHasBehaviorObject() )
		shouldDo = true;
	
	return shouldDo;
}


function _QTAddAttribute(prefix, slotName, tagName)
{
	var		value;

	value = gTagAttrs[prefix + slotName];
	if ( null == value )
		value = gTagAttrs[slotName];

	if ( null != value )
	{
		if ( 0 == slotName.indexOf(prefix) && (null == tagName) )
			tagName = slotName.substring(prefix.length); 
		if ( null == tagName ) 
			tagName = slotName;
		return ' ' + tagName + '="' + value + '"';
	}
	else
		return "";
}

function _QTAddObjectAttr(slotName, tagName)
{
	// don't bother if it is only for the embed tag
	if ( 0 == slotName.indexOf("emb#") )
		return "";

	if ( 0 == slotName.indexOf("obj#") && (null == tagName) )
		tagName = slotName.substring(4); 

	return _QTAddAttribute("obj#", slotName, tagName);
}

function _QTAddEmbedAttr(slotName, tagName)
{
	// don't bother if it is only for the object tag
	if ( 0 == slotName.indexOf("obj#") )
		return "";

	if ( 0 == slotName.indexOf("emb#") && (null == tagName) )
		tagName = slotName.substring(4); 

	return _QTAddAttribute("emb#", slotName, tagName);
}


function _QTAddObjectParam(slotName, generateXHTML)
{
	var		paramValue;
	var		paramStr = "";
	var		endTagChar = (generateXHTML) ? ' />' : '>';

	if ( -1 == slotName.indexOf("emb#") )
	{
		// look for the OBJECT-only param first. if there is none, look for a generic one
		paramValue = gTagAttrs["obj#" + slotName];
		if ( null == paramValue )
			paramValue = gTagAttrs[slotName];

		if ( 0 == slotName.indexOf("obj#") )
			slotName = slotName.substring(4); 
	
		if ( null != paramValue )
			paramStr = '<param name="' + slotName + '" value="' + paramValue + '"' + endTagChar;
	}

	return paramStr;
}

function _QTDeleteTagAttrs()
{
	for ( var ndx = 0; ndx < arguments.length; ndx++ )
	{
		var attrName = arguments[ndx];
		delete gTagAttrs[attrName];
		delete gTagAttrs["emb#" + attrName];
		delete gTagAttrs["obj#" + attrName];
	}
}


// generate an embed and object tag, return as a string
function _QTGenerate(callingFcnName, generateXHTML, args)
{
	// is the number of optional arguments even?
	if ( args.length < 4 || (0 != (args.length % 2)) )
	{
		_QTComplain(callingFcnName, gArgCountErr);
		return "";
	}
	
	// allocate an array, fill in the required attributes with fixed place params and defaults
	gTagAttrs = new Object();
	gTagAttrs["src"] = args[0];
	gTagAttrs["width"] = args[1];
	gTagAttrs["height"] = args[2];
	gTagAttrs["type"] = "video/quicktime";
	gTagAttrs["classid"] = "clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B";
		//Impportant note: It is recommended that you use this exact classid in order to ensure a seamless experience for all viewers
	gTagAttrs["pluginspage"] = "http://www.apple.com/quicktime/download/";

	// set up codebase attribute with specified or default version before parsing args so
	//  anything passed in will override
	var activexVers = args[3]
	if ( (null == activexVers) || ("" == activexVers) )
		activexVers = "7,3,0,0";
	gTagAttrs["codebase"] = "http://www.apple.com/qtactivex/qtplugin.cab#version=" + activexVers;

	var	attrName,
		attrValue;

	// add all of the optional attributes to the array
	for ( var ndx = 4; ndx < args.length; ndx += 2)
	{
		attrName = args[ndx].toLowerCase();
		attrValue = args[ndx + 1];

		gTagAttrs[attrName] = attrValue;

		if ( ("postdomevents" == attrName) && (attrValue.toLowerCase() != "false") )
		{
			gQTEventsEnabled = true;
			if ( _QTIsMSIE() )
				gTagAttrs["obj#style"] = "behavior:url(#" + gQTBehaviorID + ")";
		}
	}

	// init both tags with the required and "special" attributes
	var objTag =  '<object '
					+ _QTAddObjectAttr("classid")
					+ _QTAddObjectAttr("width")
					+ _QTAddObjectAttr("height")
					+ _QTAddObjectAttr("codebase")
					+ _QTAddObjectAttr("name")
					+ _QTAddObjectAttr("id")
					+ _QTAddObjectAttr("tabindex")
					+ _QTAddObjectAttr("hspace")
					+ _QTAddObjectAttr("vspace")
					+ _QTAddObjectAttr("border")
					+ _QTAddObjectAttr("align")
					+ _QTAddObjectAttr("class")
					+ _QTAddObjectAttr("title")
					+ _QTAddObjectAttr("accesskey")
					+ _QTAddObjectAttr("noexternaldata")
					+ _QTAddObjectAttr("obj#style")
					+ '>'
					+ _QTAddObjectParam("src", generateXHTML);
	var embedTag = '<embed '
					+ _QTAddEmbedAttr("type")
					+ _QTAddEmbedAttr("src")
					+ _QTAddEmbedAttr("width")
					+ _QTAddEmbedAttr("height")
					+ _QTAddEmbedAttr("pluginspage")
					+ _QTAddEmbedAttr("name")
					+ _QTAddEmbedAttr("id")
					+ _QTAddEmbedAttr("align")
					+ _QTAddEmbedAttr("tabindex");

	// delete the attributes/params we have already added
	_QTDeleteTagAttrs("type","src","width","height","pluginspage","classid","codebase","name","tabindex",
					"hspace","vspace","border","align","noexternaldata","class","title","accesskey","id","style");

	// and finally, add all of the remaining attributes to the embed and object
	for ( var attrName in gTagAttrs )
	{
		attrValue = gTagAttrs[attrName];
		if ( null != attrValue )
		{
			embedTag += _QTAddEmbedAttr(attrName);
			objTag += _QTAddObjectParam(attrName, generateXHTML);
		}
	} 

	// end both tags, we're done
	return objTag + embedTag + '></em' + 'bed></ob' + 'ject' + '>';
}


// return the object/embed as a string
function QT_GenerateOBJECTText()
{
	var	txt = _QTGenerate("QT_GenerateOBJECTText", false, arguments);
	if ( _QTShouldInsertBehavior() )
		txt = _QTGenerateBehavior() + txt;
	return txt;
}

function QT_GenerateOBJECTText_XHTML()
{
	var	txt = _QTGenerate("QT_GenerateOBJECTText_XHTML", true, arguments);
	if ( _QTShouldInsertBehavior() )
		txt = _QTGenerateBehavior() + txt;
	return txt;
}

function QT_WriteOBJECT()
{
	var	txt = _QTGenerate("QT_WriteOBJECT", false, arguments);
	if ( _QTShouldInsertBehavior() )
		document.writeln(_QTGenerateBehavior());
	document.writeln(txt);
}

function QT_WriteOBJECT_XHTML()
{
	var	txt = _QTGenerate("QT_WriteOBJECT_XHTML", true, arguments);
	if ( _QTShouldInsertBehavior() )
		document.writeln(_QTGenerateBehavior());
	document.writeln(txt);
}

function QT_GenerateBehaviorOBJECT()
{
	return _QTGenerateBehavior();
}

function QT_ReplaceElementContents()
{
	var element = arguments[0];
	var args = [];

	// copy all other arguments we want to pass through to the fcn
	for ( var ndx = 1; ndx < arguments.length; ndx++ )
		args.push(arguments[ndx]);

	var	txt = _QTGenerate("QT_ReplaceElementContents", false, args);
	if ( txt.length > 0 )
		element.innerHTML = txt;
}


function QT_ReplaceElementContents_XHTML()
{
	var element = arguments[0];
	var args = [];

	// copy all other arguments we want to pass through to the fcn
	for ( var ndx = 1; ndx < arguments.length; ndx++ )
		args.push(arguments[ndx]);

	var	txt = _QTGenerate("QT_ReplaceElementContents_XHTML", true, args);
	if ( txt.length > 0 )
		element.innerHTML = txt;
}

