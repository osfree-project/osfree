
/*
 *@@sourcefile xmldefs.c:
 *      this file is just for xdoc and contains glossary items for
 *      XML. It is never compiled.
 *
 *@@added V0.9.6 (2000-10-29) [umoeller]
 */

/*
 *      Copyright (C) 2001 Ulrich Mîller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

/*
 *@@gloss: expat expat
 *      Expat is one of the most well-known XML processors (parsers).
 *      I (umoeller) have ported expat to the XWorkplace Helpers
 *      library. See xmlparse.c for an introduction to expat. See
 *      xml.c for an introduction to XML support in the XWorkplace
 *      Helpers in general.
 */


/*
 *@@gloss: XML XML
 *      XML is the Extensible Markup Language, as defined by
 *      the W3C. XML isn't really a language, but a meta-language
 *      for describing markup languages. It is a simplified subset
 *      of SGML.
 *
 *      You should be familiar with the following:
 *
 *      -- XML parsers operate on XML @documents.
 *
 *      -- Each XML document has both a physical and a logical
 *         structure.
 *
 *         Physically, the document is composed of units called
 *         @entities.
 *
 *         Logically, the document is composed of @markup and
 *         @content. Among other things, markup separates the content
 *         into @elements.
 *
 *      -- The logical and physical structures must nest properly (be
 *         @well-formed) for each entity, which results in the entire
 *         XML document being well-formed as well.
 */

/*
 *@@gloss: entities entities
 *      An "entity" is an XML storage unit. It's a very abstract
 *      concept, and the term doesn't make much sense, but it was
 *      in SGML already, and XML chose to inherit it.
 *
 *      In the simplest case, an XML document has only one entity,
 *      which is an XML file (or memory buffer from wherever).
 *      The document entity serves as the root of the entity tree
 *      and a starting-point for an XML processor. Unlike other
 *      entities, the document entity has no name and might well
 *      appear on a processor input stream without any identification
 *      at all.
 *
 *      Entities are defined to be either parsed or unparsed.
 *
 *      Other than that, there are @internal_entities,
 *      @external_entities, and @parameter_entities.
 *
 *      See @entity_references for how to reference entities.
 */

/*
 *@@gloss: entity_references entity references
 *      An "entity reference" refers to the content of a named
 *      entity (see: @entities). It is included in "&amp" and ";"
 *      characters.
 *
 *      If you declare @internal_entities in the @DTD, referencing
 *      them allows for text replacements as in SGML:
 *
 +              This document was prepared on &PrepDate;.
 *
 *      The same works for @external_entities though. Assuming
 *      that "SecondFile" has been declared in the DTD to point
 *      to another file,
 *
 +              See the following README: &SecondFile;
 *
 *      would then insert the complete contents of the second
 *      file into the document. The XML processor will parse
 *      that file as if it were at that position in the original
 *      document.
 *
 *      An entity is "included" when its replacement text
 *      is retrieved and processed, in place of the reference itself,
 *      as though it were part of the document at the location the
 *      reference was recognized.
 *      The replacement text may contain
 *      both @content and (except for @parameter_entities)
 *      @markup, which must be recognized in the usual way, except
 *      that the replacement text of entities used to escape markup
 *      delimiters (the entities amp, lt, gt, apos, quot) is always
 *      treated as data. (The string "AT&amp;T;" expands to "AT&T;"
 *      and the remaining ampersand is not recognized as an
 *      entity-reference delimiter.) A @character_reference is
 *      included when the indicated character is processed in
 *      place of the reference itself.
 *
 *      The following are forbidden, and constitute fatal errors:
 *
 *      --  the appearance of a reference to an unparsed entity;
 *
 *      --  the appearance of any character or general-entity reference
 *          in the @DTD except within an EntityValue or AttValue;
 *
 *      --  a reference to an external entity in an attribute value.
 */

/*
 *@@gloss: internal_entities internal entities
 *      An "internal entity" has no separate physical storage.
 *      Its contents appear in the document's @DTD as an
 *      @entity_declaration, like this:
 *
 +              <!ENTITY PrepDate "Feb 11, 2001">
 *
 *      This can later be referenced with @entity_references
 *      and allows you to define shortcuts for frequently typed
 *      text or text that is expected to change, such as the
 *      revision status of a document.
 *
 *      XML has five built-in internal entities:
 *
 *      --  "&amp;amp;" refers to the ampersand ("&amp") character,
 *          which normally introduces @markup and can therefore
 *          only be literally used in @comments, @processing_instructions,
 *          or @CDATA sections. This is also legal within the literal
 *          entity value of declarations of internal entities.
 *
 *      --  "&amp;lt;" and "&amp;gt;" refer to the angle brackets
 *          ("&lt;", "&gt;") which normally introduce @elements.
 *          They must be escaped unless used in a @CDATA section.
 *
 *      --  To allow values in @attributes to contain both single and double
 *          quotes, the apostrophe or single-quote character (') may be
 *          represented as "&amp;apos;", and the double-quote character
 *          (") as "&amp;quot;".
 *
 *      A numeric @character_reference is a special case of an entity reference.
 *
 *      An internal entity is always parsed.
 *
 *      Also see @entities.
 */

/*
 *@@gloss: parameter_entities parameter entities
 *      Parameter entities can only be references in the @DTD.
 *      A parameter entity is identified by placing "% " (percent-space)
 *      in front of its name in the declaration. The percent sign is
 *      also used in references to parameter entities, instead of the
 *      ampersand. Parameter entity references are immediately expanded
 *      in the DTD and their replacement text is
 *      part of the declaration, whereas normal @entity_references are not
 *      expanded.
 */

/*
 *@@gloss: external_entities external entities
 *      As opposed to @internal_entities, "external entities" refer
 *      to different storage.
 *
 *      They must have a "system ID" with the URI specifying where
 *      the entity can be retrieved. Those URIs may be absolute
 *      or relative. Unless otherwise provided (e.g. by a special
 *      XML element type defined by a particular @DTD, or
 *      @processing_instructions defined by a particular application
 *      specification), relative URIs are relative to the location
 *      of the resource within which the entity declaration occurs.
 *
 *      Optionally, external entities may specify a "public ID"
 *      as well. An XML processor attempting to retrieve the entity's
 *      content may use the public identifier to try to generate an
 *      alternative URI. If the processor is unable to do so, it must
 *      use the URI specified in the system literal. Before a match
 *      is attempted, all strings of @whitespace in the public
 *      identifier must be normalized to single space characters (#x20),
 *      and leading and trailing white space must be removed.
 *
 *      An external entity is not always parsed.
 *
 *      External entities allow an XML document to refer to an external
 *      file. External entities contain either text or binary data. If
 *      they contain text, the content of the external file is inserted
 *      at the point of reference and parsed as part of the referring
 *      document. Binary data is not parsed and may only be referenced
 *      in an attribute that has been declared as ENTITY or ENTITIES.
 *      Binary data is used to reference figures and
 *      other non-XML content in the document.
 *
 *      Examples of external entity declarations:
 +
 +         <!ENTITY open-hatch
 +                   SYSTEM "http://www.textuality.com/boilerplate/OpenHatch.xml">
 +         <!ENTITY open-hatch
 +                   PUBLIC "-//Textuality//TEXT Standard open-hatch boilerplate//EN"
 +                   "http://www.textuality.com/boilerplate/OpenHatch.xml">
 +         <!ENTITY hatch-pic
 +                   SYSTEM "../grafix/OpenHatch.gif" NDATA gif >
 *
 *      Character @encoding is processed on a per-external-entity basis.
 *      As a result, each external parsed entity in an XML document may
 *      use a different encoding for its characters.
 *
 *      In the document entity, the encoding declaration is part of the
 *      XML @text_declaration.
 *
 *      Also see @entities.
 */

/*
 *@@gloss: external_parsed_entities external parsed entities
 *      An external parsed entity is an external entity that has
 *      been parsed, which is not necessarily the case.
 *
 *      See @external_entities.
 */

/*
 *@@gloss: markup markup
 *      XML "markup" encodes a description of the @document's storage
 *      layout and logical structure.
 *
 *      Markup is either @elements, @entity_references, @comments, @CDATA
 *      section delimiters, @DTD's, or @processing_instructions.
 *
 *      XML "text" consists of markup and @content.
 */

/*
 *@@gloss: whitespace whitespace
 *      In XML, "whitespace" consists of one or more space (0x20)
 *      characters, carriage returns, line feeds, or tabs.
 *
 *      Whitespace handling in XML can vary. In @markup, this is
 *      used to separate the various @entities of course. However,
 *      in @content (i.e. non-markup), an application may
 *      or may not be interested in white space. Whitespace
 *      handling can therefore be handled differently for each
 *      element with the use of the special "xml:space" @attributes.
 */

/*
 *@@gloss: character_reference character reference
 *      Character references escape Unicode characters. They are
 *      a special case of @entity_references.
 *
 *      They may be used to refer to a specific character in the
 *      ISO/IEC 10646 character set, for example one not directly
 *      accessible from available input devices.
 *
 *      If the character reference thus begins with "&amp;#x", the
 *      digits and letters up to the terminating ";" provide a
 *      hexadecimal representation of the character's code point in
 *      ISO/IEC 10646. If it begins just with "&amp;#", the
 *      digits up to the terminating ";" provide a decimal
 *      representation of the character's code point.
 */

/*
 *@@gloss: content content
 *      XML "text" consists of @markup and "content" (the XML spec
 *      calls this "character data"). Content is simply everything
 *      that is not markup.
 *
 *      To access characters that would either otherwise be recognized
 *      as @markup or are difficult to reach via the keyboard, XML
 *      allows for using a @character_reference.
 *
 *      Within @elements, content is any string of
 *      characters which does not contain the start-delimiter of
 *      any markup. In a @CDATA section, content is any
 *      string of characters not including the CDATA-section-close
 *      delimiter, "]]>".
 *
 *      The character @encodings may vary between @external_parsed_entities.
 */

/*
 *@@gloss: names names
 *      In XML, a "name" is a token beginning with a letter or one of a
 *      few punctuation characters, and continuing with letters,
 *      digits, hyphens, underscores, colons, or full stops,
 *      together known as name characters. The colon has a
 *      special meaning with XML namespaces.
 */

/*
 *@@gloss: elements elements
 *      Elements are the most common form of XML @markup.
 *      They are identified by their @names.
 *
 *      As opposed to HTML, there are two types of elements:
 *
 *      A non-empty element starts and ends with a start-tag
 *      and an end-tag:
 *
 +           <LI>...</LI>
 *
 *      As opposed to HTML, an empty element must have an
 *      empty-element tag:
 *
 +           <P /> <IMG align="left" src="http://www.w3.org/Icons/WWW/w3c_home" />
 *
 *      In addition, @attributes contains extra parameters to elements.
 *      If the element has attributes, they must be in the start-tag
 *      (or empty-element tag).
 *
 *      For non-empty elements, the text between the start-tag
 *      and end-tag is called the element's content and may
 *      contain other elements, character data, an entity
 *      reference, a @CDATA section, a processing instruction,
 *      or a comment.
 *
 *      The XML specs break this into "content particles".
 *
 *      An element has "mixed content" when it may contain
 *      @content, optionally interspersed with child
 *      elements. In this case, the types of the child
 *      elements may be constrained by a documents @DTD, but
 *      not their order or their number of occurrences.
 */

/*
 *@@gloss: attributes attributes
 *      "Attributes" are name-value pairs that have been associated
 *      with @elements. Attributes can only appear in start-tags
 *      or empty-tags.
 *
 *      Attributes are identified by their @names. Each such
 *      identifier may only appear once per element.
 *
 *      As opposed to HTML, attribute values must be quoted (either
 *      in single or double quotes). You may use a @character_reference
 *      to escape quotes in attribute values.
 *
 *      Example of an attribute:
 *
 +          <IMG SRC="mypic.gif" />
 *
 *      SRC="mypic.gif" is the attribute here.
 *
 *      There are a few <B>special attributes</B> defined by XML.
 *      In @valid documents, these attributes, like any other,
 *      must be declared if they are used. These attributes are
 *      recursive, i.e. they are considered to apply to all elements
 *      within the content of the element where they are specified,
 *      unless overridden in a sub-element.
 *
 *      -- "xml:space" may be attached to an element to signal
 *         that @whitespace should be preserved for this element.
 *
 *         The value "default" signals that applications' default
 *         whitespace processing modes are acceptable for this
 *         element; the value "preserve" indicates the intent that
 *         applications preserve all the white space.
 *
 *      -- "xml:lang" may be inserted in documents to specify the
 *         language used in the contents and attribute values of
 *         any element in an XML document.
 *
 *         The value is either a two-letter language code (e.g. "en")
 *         or a combination of language and country code. Interestingly,
 *         the English W3C XML spec gives the following examples:
 *
 +         <p xml:lang="en">The quick brown fox jumps over the lazy dog.</p>
 +         <p xml:lang="en-GB">What colour is it?</p>
 +         <p xml:lang="en-US">What color is it?</p>
 +         <sp who="Faust" desc='leise' xml:lang="de">
 +              <l>Habe nun, ach! Philosophie,</l>
 +              <l>Juristerei, und Medizin</l>
 +              <l>und leider auch Theologie</l>
 +              <l>durchaus studiert mit hei·em BemÅh'n.</l>
 +          </sp>
 */

/*
 *@@gloss: comments comments
 *      Comments may appear anywhere in a document outside other
 *      markup; in addition, they may appear within the @DTD at
 *      places allowed by the grammar. They are not part of the
 *      document's @content; an XML processor may, but
 *      need not, make it possible for an application to retrieve
 *      the text of comments (@expat has a handler for this).
 *
 *      Comments may contain any text except "--" (double-hyphen).
 *
 *      Example of a comment:
 *
 +          <!-- declarations for <head> & <body> -->
 */

/*
 *@@gloss: CDATA CDATA
 *      CDATA sections can appear anywhere where @content
 *      is allowed. They are used to escape blocks of
 *      text containing characters which would otherwise be
 *      recognized as @markup.
 *
 *      CDATA sections begin with the string &lt;![CDATA[ and end
 *      with the string ]]&gt;. Within a CDATA section, only the
 *      ]]&gt; string is recognized as @markup, so that left angle
 *      brackets and ampersands may occur in their literal form.
 *      They need not (and cannot) be escaped using "&amp;lt;" and
 *      "&amp;amp;". (This implies that not even @comments are
 *      recognized).
 *
 *      CDATA sections cannot nest.
 *
 *      Examples:
 *
 +          <![CDATA[<greeting>Hello, world!</greeting>]]>
 +
 +          <![CDATA[
 +          *p = &q;
 +          b = (i <= 3);
 +          ]]>
 */

/*
 *@@gloss: processing_instructions processing instructions
 *      "Processing instructions" (PIs) contain additional
 *      data for applications.
 *
 *      Like @comments, they are not textually part of the XML
 *      document, but the XML processor is required to pass
 *      them to an application.
 *
 *      PIs have the form:
 *
 +          <?name pidata?>
 *
 *
 *      The "name", called the PI "target", identifies the PI to
 *      the application. Applications should process only the
 *      targets they recognize and ignore all other PIs. Any
 *      data that follows the PI target is optional, it is for
 *      the application that recognizes the target. The names
 *      used in PIs may be declared in a @notation_declaration in order to
 *      formally identify them.
 *
 *      PI names beginning with "xml" are reserved.
 */

/*
 *@@gloss: well-formed well-formed
 *      XML @documents (the sum of all @entities) are "well-formed"
 *      if the following conditions are met (among others):
 *
 *      -- They contain one or more @elements.
 *
 *      -- There is exactly one element, called the root, or document
 *         element, no part of which appears in the @content of any
 *         other element.
 *
 *      -- For all other elements, if the start-tag is in the content
 *         of another element, the end-tag is in the content of the
 *         same element. More simply stated, the elements nest
 *         properly within each other. (This is unlike HTML.)
 *
 *      -- Values of string @attributes cannot contain references to
 *         @external_entities.
 *
 *      -- No attribute may appear more than once in the same element.
 *
 *      -- All entities except the amp, lt, gt, apos, and quot must be
 *         declared before they are used. Binary @external_entities
 *         cannot be referenced in the flow of @content, it can only
 *         be used in an attribute declared as ENTITY or ENTITIES.
 *
 *      -- Neither text nor @parameter_entities are allowed to be
 *         recursive, directly or indirectly.
 */

/*
 *@@gloss: valid valid
 *      XML @documents are said to be "valid" if they have a @DTD
 *      associated and they confirm to it. While XML documents
 *      must always be @well-formed, validation and validity is up
 *      to the implementation (i.e. at option to the application).
 *
 *      Validating processors must report violations of the constraints
 *      expressed by the declarations in the @DTD, and failures to
 *      fulfill the validity constraints given in this specification.
 *      To accomplish this, validating XML processors must read and
 *      process the entire DTD and all @external_parsed_entities
 *      referenced in the document.
 *
 *      Non-validating processors (such as @expat) are required to
 *      check only the document entity (see @entitites), including the
 *      entire internal DTD subset, for whether it is @well-formed.
 *
 *      While they are  not required to check the document for validity,
 *      they are required to process all the declarations they
 *      read in the internal DTD subset and in any parameter entity
 *      that they read, up to the first reference to a parameter
 *      entity that they do not read; that is to say, they must
 *      use the information in those declarations to normalize
 *      values of @attributes, include the replacement text of
 *      @internal_entities, and supply default attribute values.
 *      They must not process entity declarations or attribute-list
 *      declarations encountered after a reference to a
 *      parameter entity that is not read, since the entity may have
 *      contained overriding declarations.
 */

/*
 *@@gloss: encodings encodings
 *      XML supports a wide variety of character encodings. These
 *      must be specified in the XML @text_declaration.
 *
 *      There are too many character encodings on the planet to
 *      be listed here. The most common ones are:
 *
 *      --  "UTF-8", "UTF-16", "ISO-10646-UCS-2", and "ISO-10646-UCS-4"
 *          should be used for the various encodings and transformations
 *          of Unicode / ISO/IEC 10646.
 *
 *      --  "ISO-8859-x" (with "x" being a number from 1 to 9) represent
 *          the various ISO 8859 ("Latin") encodings.
 *
 *      --  "ISO-2022-JP", "Shift_JIS", and "EUC-JP" should be used for
 *          the various encoded forms of JIS X-0208-1997.
 *
 *      Example of a @text_declaration:
 *
 +          <?xml version="1.0" encoding="ISO-8859-2"?>
 *
 *      All XML processors must be able to read @entities in either
 *      UTF-8 or UTF-16. @expat directly supports the following
 *      (see XML_SetUnknownEncodingHandler):
 *
 *      --   UTF-8: 8-bit encoding of Unicode.
 *
 *      --   UTF-16: 16-bit encoding of Unicode.
 *
 *      --   ISO-8859-1: that's "latin 1".
 *
 *      --   US-ASCII.
 *
 *      Entities encoded in UTF-16 must begin with the ZERO WIDTH NO-BREAK
 *      SPACE character, #xFEFF). This is an encoding signature, not part
 *      of either the @markup or the @content of the XML @document.
 *      XML processors must be able to use this character to differentiate
 *      between UTF-8 and UTF-16 encoded documents.
 */

/*
 *@@gloss: text_declaration text declaration
 *      XML @documents and  @external_parsed_entities may (and
 *      should) start with the XML text declaration, exactly like
 *      this:
 *
 +          <?xml version="1.0" encoding="enc"?>
 *
 *      where "1.0" is the only currently defined XML version
 *      and "enc" must be the encoding of the document.
 *
 *      External parsed entities may begin with a text declaration,
 *      which looks like an XML declaration with just an encoding
 *      declaration:
 *
 +          <?xml encoding="Big5"?>
 *
 *      See @encodings.
 *
 *      Example:
 *
 +          <?xml version="1.0" encoding="ISO-8859-1"?>
 */

/*
 *@@gloss: documents documents
 *      XML documents are made up of storage units called @entities,
 *      which contain either parsed or unparsed data. Parsed data is
 *      made up of characters, some of which form @content,
 *      and some of which form @markup.
 *
 *      XML documents should start the with the XML @text_declaration.
 *
 *      The function of the @markup in an XML document is to describe
 *      its storage and logical structure and to associate attribute-value
 *      pairs with its logical structures. XML provides a mechanism,
 *      the document type declaration (@DTD), to define constraints
 *      on the logical structure and to support the use of predefined
 *      storage units.
 *
 *      A data object is an XML document if it is @well-formed.
 *      A well-formed XML document may in addition be @valid if it
 *      meets certain further constraints.
 *
 *      A very simple XML document looks like this:
 *
 +          <?xml version="1.0"?>
 +          <oldjoke>
 +          <burns>Say <quote>goodnight</quote>, Gracie.</burns>
 +          <allen><quote>Goodnight, Gracie.</quote></allen>
 +          <applause/>
 +          </oldjoke>
 *
 *      This document is @well-formed, but not @valid (because it
 *      has no @DTD).
 *
 */

/*
 *@@gloss: element_declaration element declaration
 *      Element declarations identify the @names of elements and the
 *      nature of their content. They look like this:
 +
 +          <!ELEMENT name contentspec>
 +
 *      No element may be declared more than once.
 *
 *      The "name" of the element is obvious. The "contentspec"
 *      is not. This specifies what may appear in the element
 *      and can be one of the following:
 *
 *      --  "EMPTY" marks the element as being empty (i.e.
 *          having no content at all).
 *
 *      --  "ANY" does not impose any restrictions.
 *
 *      --  (mixed): a "list" which declares the element to have
 *          mixed content. See below.
 *
 *      --  (children): a "list" which declares the element to
 *          have child elements only, but no content. See below.
 *
 *      <B>(mixed): content with elements</B>
 *
 *      With the (mixed) contentspec, an element may either contain
 *      @content only or @content with subelements.
 *
 *      While the (children) contentspec allows you to define sequences
 *      and orders, this is not possible with (mixed).
 *
 *      "contentspec" must then be a pair of parentheses, optionally
 *      followed by "*". In the brackets, there must be at least the
 *      keyword "#PCDATA", optionally followed by "|" and element
 *      names. Note that if no #PCDATA appears, the (children) model
 *      is assumed (see below).
 *
 *      Examples:
 *
 +          <!ELEMENT name (#PCDATA)* >
 +          <!ELEMENT name (#PCDATA | subname1 | subname2)* >
 +          <!ELEMENT name (#PCDATA) >
 *
 *      Note that if you specify sub-element names, you must terminate
 *      the contentspec with "*". Again, there's no way to specify
 *      orders etc. with (mixed).
 *
 *      <B>(children): Element content only</B>
 *
 *      With the (children) contentspec, an element may contain
 *      only other elements (and @whitespace), but no other @content.
 *
 *      This can become fairly complicated. "contentspec" then must be
 *      a "list" followed by a "repeater".
 *
 *      A "repeater" can be:
 *
 *      --  Nothing: the preceding item _must_ appear exactly once.
 *
 *      --  "+": the preceding item _must_ appear at _least_ once.
 *
 *      --  "?": the preceding item _may_ appear exactly once.
 *
 *      --  "*": the preceding item _may_ appear once or more than
 *          once or not at all.
 *
 *      Here's the most simple example (precluding that "SUBELEMENT"
 *      is a valid "list" here):
 *
 +          <!ELEMENT name (SUBELEMENT)* >
 *
 *      In other words, in (children) mode, "contentspec" must always
 *      be in brackets and is followed by a "repeater" (which can be
 *      nothing).
 *
 *      About "lists"... since these declarations may nest, this is
 *      where the recursive definition of a "content particle" comes
 *      in:
 *
 *      --  A "content particle" is either a sub-element name or
 *          a nested list, followed by a "repeater".
 *
 *      --  A "list" is defined as an enumeration of content particles,
 *          enclosed in parentheses, where the content particles are
 *          separated by "connectors".
 *
 *      There are two types of "connectors":
 *
 *      --  Commas (",") indicate that the elements must appear
 *          in the specified order ("sequence").
 *
 *      --  Vertical bars ("|") specify that the elements may
 *          occur alternatively ("choice").
 *
 *      The connectors cannot be mixed; the list must be
 *      either completely "sequence" or "choice".
 *
 *      Examples of content particles:
 *
 +              SUBELEMENT+
 +              list*
 *
 *      Examples of lists:
 *
 +          ( cp | cp | cp | cp )
 +          ( cp , cp , cp , cp )
 *
 *      Full examples for (children):
 *
 +          <!ELEMENT oldjoke  ( burns+, allen, applause? ) >
 +                             | |       +cp-+          | |
 +                             | |                      | |
 +                             | +------- list ---------+ |
 +                             +-------contentspec--------+
 *
 *      This specifies a "seqlist" for the "oldjoke" element. The
 *      list is not nested, so the content particles are element
 *      names only.
 *
 *      Within "oldjoke", "burns" must appear first and can appear
 *      once or several times.
 *
 *      Next must be "allen", exactly once (since there's no repeater).
 *
 *      Optionally ("?"), there can be "applause" at the end.
 *
 *      Now, a nested example:
 *
 +          <!ELEMENT poem (title?, (stanza+ | couplet+ | line+) ) >
 *
 *      That is, a poem consists of an optional title, followed by one or
 *      several stanzas, or one or several couplets, or one or several lines.
 *      This is different from:
 *
 +          <!ELEMENT poem (title?, (stanza | couplet | line)+ ) >
 *
 *      The latter allows for a single poem to contain a mixture of stanzas,
 *      couplets or lines.
 *
 *      And for WarpIN:
 *
 +          <!ELEMENT WARPIN (REXX*, VARPROMPT*, MSG?, TITLE?, (GROUP | PCK)+), PAGE+) >
 *
 */

/*
 *@@gloss: attribute_declaration attribute declaration
 *      Attribute declarations identify the @names of attributes
 *      of @elements and their possible values. They look like this:
 *
 +      <!ATTLIST elementname
 +           attname   atttype   defaultvalue
 +           attname   atttype   defaultvalue
 +              ... >
 *
 *      "elementname" is the element name for which the
 *      attributes are being defined.
 *
 *      For each attribute, you must then specify three
 *      columns:
 *
 *      --  "attname" is the attribute name.
 *
 *      --  "atttype" is the attribute type (one of six values,
 *          see below).
 *
 *      --  "defaultvalue" specifies the default value.
 *
 *      The attribute type (specifying the value type) must be
 *      one of six:
 *
 *      --  "CDATA" is any character data. (This has nothing to
 *          do with @CDATA sections.)
 *
 *      --  "ID": the value must be a unique @name among the
 *          document. Only one such attribute is allowed per
 *          element.
 *
 *      --  "IDREF" or "IDREFS": a reference to some other
 *          element which has an "ID" attribute with this value.
 *          "IDREFS" is the plural and may contain several of
 *          those separated by @whitespace.
 *
 *      --  "ENTITY" or "ENTITIES": a reference to some an
 *          external entity (see @external_entities).
 *          "ENTITIES" is the plural and may contain several of
 *          those separated by @whitespace.
 *
 *      --  "NMTOKEN" or "NMTOKENS": a single-word string.
 *          This is not a reference though.
 *          "NMTOKENS" is the plural and may contain several of
 *          those separated by @whitespace.
 *
 *      --  an enumeration: an explicit list of allowed
 *          values for this attribute. Additionally, you can specify
 *          that the names must match a particular @notation_declaration.
 *
 *      The "defaultvalue" (third column) can be one of these:
 *
 *      --  "#REQUIRED": the attribute may not be omitted.
 *
 *      --  "#IMPLIED": the attribute is optional, and there's
 *          no default value.
 *
 *      --  "'value'": the attribute is optional, and it has
 *          this default.
 *
 *      --  "#FIXED 'value'": the attribute is optional, but if
 *          it appears, it must have this value.
 *
 *      Example:
 *
 +      <!ATTLIST oldjoke
 +           name   ID                #REQUIRED
 +           label  CDATA             #IMPLIED
 +           status ( funny | notfunny ) 'funny'>
 */

/*
 *@@gloss: entity_declaration entity declaration
 *      Entity declarations define @entities.
 *
 *      An example of @internal_entities:
 *
 +          <!ENTITY ATI             "ArborText, Inc.">
 *
 *      Examples of @external_entities:
 *
 +          <!ENTITY boilerplate     SYSTEM "/standard/legalnotice.xml">
 +          <!ENTITY ATIlogo         SYSTEM "/standard/logo.gif" NDATA GIF87A>
 */

/*
 *@@gloss: notation_declaration notation declaration
 *      Notation declarations identify specific types of external
 *      binary data. This information is passed to the processing
 *      application, which may make whatever use of it it wishes.
 *
 *      Example:
 *
 +          <!NOTATION GIF87A SYSTEM "GIF">
 */

/*
 *@@gloss: DTD DTD
 *      The XML document type declaration contains or points to
 *      markup declarations that provide a grammar for a class of @documents.
 *      This grammar is known as a Document Type Definition, or DTD.
 *
 *      The DTD must look like the following:
 *
 +              <!DOCTYPE name ... >
 *
 *      "name" must match the document's root element.
 *
 *      "..." can be the reference to an external subset (being a special
 *      case of @external_entities):
 *
 +              <!DOCTYPE name SYSTEM "whatever.dtd">
 *
 *      The SYSTEM identifier is required with XML, while a public
 *      identifier is not. (In SGML, neither is required, but at
 *      least one must be present.)
 *
 *      Alternatively,specify an internal subset in brackets, which
 *      contains the markup directly:
 *
 +              <!DOCTYPE name [
 +                      <!ELEMENT greeting (#PCDATA)>
 +              ]>
 *
 *      You can even mix both.
 *
 *      A markup declaration is either an @element_declaration, an
 *      @attribute_declaration, an @entity_declaration,
 *      or a @notation_declaration. These declarations may be contained
 *      in whole or in part within @parameter_entities.
 */

/*
 *@@gloss: DOM DOM
 *      DOM is the "Document Object Model", as defined by the W3C.
 *
 *      The DOM is a programming interface for @XML @documents.
 *      (XML is a metalanguage and describes the documents
 *      themselves. DOM is a programming interface -- an API --
 *      to access XML documents.)
 *
 *      The W3C calls this "a platform- and language-neutral
 *      interface that allows programs and scripts to dynamically
 *      access and update the content, structure and style of
 *      documents. The Document Object Model provides
 *      a standard set of objects for representing HTML and XML
 *      documents, a standard model of how these objects can
 *      be combined, and a standard interface for accessing and
 *      manipulating them. Vendors can support the DOM as an
 *      interface to their proprietary data structures and APIs,
 *      and content authors can write to the standard DOM
 *      interfaces rather than product-specific APIs, thus
 *      increasing interoperability on the Web."
 *
 *      In short, DOM specifies that an XML document is broken
 *      up into a tree of "nodes", representing the various parts
 *      of an XML document. Such nodes represent @documents,
 *      @elements, @attributes, @processing_instructions,
 *      @comments, @content, and more.
 *
 *      See xml.c for an introduction to XML and DOM support in
 *      the XWorkplace helpers.
 *
 *      Example: Take this HTML table definition:
 +
 +          <TABLE>
 +          <TBODY>
 +          <TR>
 +          <TD>Column 1-1</TD>
 +          <TD>Column 1-2</TD>
 +          </TR>
 +          <TR>
 +          <TD>Column 2-1</TD>
 +          <TD>Column 2-2</TD>
 +          </TR>
 +          </TBODY>
 +          </TABLE>
 *
 *      In the DOM, this would be represented by a tree as follows:
 +
 +                          ⁄ƒƒƒƒƒƒƒƒƒƒƒƒø
 +                          ≥   TABLE    ≥        (only ELEMENT node in root DOCUMENT node)
 +                          ¿ƒƒƒƒƒ¬ƒƒƒƒƒƒŸ
 +                                ≥
 +                          ⁄ƒƒƒƒƒ¡ƒƒƒƒƒƒø
 +                          ≥   TBODY    ≥        (only ELEMENT node in root "TABLE" node)
 +                          ¿ƒƒƒƒƒ¬ƒƒƒƒƒƒŸ
 +                    ⁄ƒƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒƒø
 +              ⁄ƒƒƒƒƒ¡ƒƒƒƒƒƒø          ⁄ƒƒƒƒƒ¡ƒƒƒƒƒƒø
 +              ≥   TR       ≥          ≥   TR       ≥
 +              ¿ƒƒƒƒƒ¬ƒƒƒƒƒƒŸ          ¿ƒƒƒƒƒ¬ƒƒƒƒƒƒŸ
 +                ⁄ƒƒƒ¡ƒƒƒƒƒƒø            ⁄ƒƒƒ¡ƒƒƒƒƒƒø
 +            ⁄ƒƒƒ¡ƒø     ⁄ƒƒ¡ƒƒø     ⁄ƒƒƒ¡ƒø     ⁄ƒƒ¡ƒƒø
 +            ≥ TD  ≥     ≥ TD  ≥     ≥ TD  ≥     ≥ TD  ≥
 +            ¿ƒƒ¬ƒƒŸ     ¿ƒƒ¬ƒƒŸ     ¿ƒƒƒ¬ƒŸ     ¿ƒƒ¬ƒƒŸ
 +         …ÕÕÕÕÕ ÕÕÕÕª …ÕÕÕÕ ÕÕÕÕÕª …ÕÕÕÕ ÕÕÕÕÕª …ÕÕ ÕÕÕÕÕÕÕª
 +         ∫Column 1-1∫ ∫Column 1-2∫ ∫Column 2-1∫ ∫Column 2-2∫    (one TEXT node in each parent node)
 +         »ÕÕÕÕÕÕÕÕÕÕº »ÕÕÕÕÕÕÕÕÕÕº »ÕÕÕÕÕÕÕÕÕÕº »ÕÕÕÕÕÕÕÕÕÕº
 */

/*
 *@@gloss: DOM_DOCUMENT DOCUMENT
 *      representation of XML @documents in the @DOM.
 *
 *      The xwphelpers implementation has the following differences
 *      to the DOM specs:
 *
 *      -- The "doctype" member points to the documents @DTD, or is NULL.
 *         In our implementation, this is the pvExtra pointer, which points
 *         to a _DOMDTD.
 *
 *      -- The "implementation" member points to a DOMImplementation object.
 *         This is not supported here.
 *
 *      -- The "documentElement" member is a convenience pointer to the
 *         document's root element. We don't supply this field; instead,
 *         the llChildren list only contains a single ELEMENT node for the
 *         root element.
 *
 *      -- The "createElement" method is implemented by xmlCreateElementNode.
 *
 *      -- The "createAttribute" method is implemented by xmlCreateAttributeNode.
 *
 *      -- The "createTextNode" method is implemented by xmlCreateTextNode,
 *         which has an extra parameter though.
 *
 *      -- The "createComment" method is implemented by xmlCreateCommentNode.
 *
 *      -- The "createProcessingInstruction" method is implemented by
 *         xmlCreatePINode.
 *
 *      -- The "createDocumentFragment", "createCDATASection", and
 *         "createEntityReference" methods are not supported.
 */


