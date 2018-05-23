<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" indent="no" encoding="UTF-8"/>

  <xsl:template match="/VisualStudioProject"><xsl:apply-templates/></xsl:template>

  <xsl:template match="/VisualStudioProject/Files/File">
    <xsl:value-of select="substring(@RelativePath,1,100)"/> \
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="/VisualStudioProject/Files/Filter/File">
    <xsl:value-of select="substring(@RelativePath,1,100)"/> \
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="/VisualStudioProject/Files/Filter/Filter/File">
    <xsl:value-of select="substring(@RelativePath,1,100)"/> \
    <xsl:apply-templates/>
  </xsl:template>

</xsl:stylesheet>
