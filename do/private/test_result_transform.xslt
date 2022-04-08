<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" />

  <xsl:template match="/testsuites">
    <TestRun>
      <FailedTests>
        <xsl:for-each select="testsuite">
          <xsl:for-each select="testcase">
            <xsl:variable name="classname" select="@classname" />
            <xsl:variable name="name" select="@name" />
            <xsl:choose>
              <xsl:when test="failure">
                <FailedTest>
                  <Name>\<xsl:value-of select="concat($classname,'::',$name)" /></Name>
                  <FailureType> Assertion</FailureType>
                  <Location>
                    <xsl:variable name="location" select="substring-before(failure/@message,'&#x0A;')" />
                    <File><xsl:value-of select="substring-before($location,':')" /></File>
                    <Line><xsl:value-of select="substring-after($location,':')" /></Line>
                  </Location>
                  <Message>
                    <xsl:variable name="warning" select="substring-after(failure/@message,'&#x0A;')" />
                    <xsl:value-of select="$warning" />
                  </Message>
                </FailedTest>
              </xsl:when>
            </xsl:choose>
          </xsl:for-each>
        </xsl:for-each>
      </FailedTests>
      <SuccessfulTests>
        <xsl:for-each select="testsuite">
          <xsl:for-each select="testcase">
            <xsl:variable name="classname" select="@classname" />
            <xsl:variable name="name" select="@name" />
            <xsl:choose>
              <xsl:when test="failure" />
              <xsl:otherwise>
                <Test>
                  <Name><xsl:value-of select="concat($classname,'::',$name)" /></Name>
                </Test>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:for-each>
        </xsl:for-each>
      </SuccessfulTests>
      <Statistics>
        <Tests><xsl:value-of select="@tests" /></Tests>
        <FailuresTotal><xsl:value-of select="@failures" /></FailuresTotal>
        <Errors><xsl:value-of select="@errors" /></Errors>
        <Failures><xsl:value-of select="@failures" /></Failures>
      </Statistics>
    </TestRun>
  </xsl:template>
</xsl:stylesheet>
