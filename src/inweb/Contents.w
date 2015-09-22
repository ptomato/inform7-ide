Title: inweb
Author: Graham Nelson
Purpose: A simple literate programming tool intended for medium-sized and large programs.
Language: C for Inform
Declare Section Usage: Off
Licence: This is a free, open-source program published under the Artistic License 2.0.
Build Number: 7/151208 ('Escape to Danger')
Build Date: December 2015

Import: inlib

Preliminaries
	The inweb Manual
	Inweb vs Cweb

Chapter 1: Top Level
"Dealing with the user, and deciding what is to be done."
	Basics
	Program Control
	Configuration

Chapter 2: Parsing a Web
"Reading in the entire text of the web, parsing its structure and looking for
identifier names within it."
	Bibliographic Data
	Line Categories
	Reading Sections
	The Parser
	Paragraph Numbering

Chapter 3: Outputs
"Either weaving part or all of the web into a typeset form for human eyes
(or a swarm of many such parts), or tangling the web into an executable program,
or analysing the web to provide diagnostics on it."
	The Analyser
	The Swarm
	The Weaver
	The Tangler

Chapter 4: Languages
"Providing support for syntax-colouring and for better organisation of code
in different programming languages."
	Programming Languages
	C-Like Languages
	C for Inform

Chapter 5: Formats
"Weaving to a variety of different human-readable formats."
	Weave Formats
	Plain Text Format
	TeX Format
	HTML Formats
	Running Through TeX
