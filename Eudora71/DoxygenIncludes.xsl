<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" indent="no" encoding="UTF-8"/>

  <xsl:template match="/VisualStudioProject"><xsl:apply-templates/></xsl:template>

  <xsl:template match="/VisualStudioProject/Configurations/Configuration[1]/Tool">
    <xsl:value-of select="@AdditionalIncludeDirectories"/>
  </xsl:template>

</xsl:stylesheet>
