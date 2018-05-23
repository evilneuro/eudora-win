#ifndef TEXT2HTML_H
#define TEXT2HTML_H

CString	Text2Html(LPCTSTR szSource, BOOL bConvertUrls, BOOL bProcessAttachmentAndPlugins, BOOL bConvertQuotesToExcerpt = FALSE);

//The following needed for Auto URLS in Paige
void InitHtmlStrings();
BOOL IsURLScheme(const char* Text);

#endif
