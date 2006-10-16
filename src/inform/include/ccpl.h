! ---------------------------------------------------------------------------- !
!   CCPL.h for Inform 6, by Roger Firth (roger@firthworks.com)
!
!   1.2 (May 03)
!       correction of "provied" typo.
!
!   1.1 (May 03)
!       use of Menus.h is made optional.
!
!   1.0 (May03)
!       original version.
!
! ---------------------------------------------------------------------------- !
!   Installation: add the lines:
!
!       Include "Menus";    ! optional
!       Include "CCPL";
!
!   anywhere in your game AFTER the Include "VerbLib"; directive, and modify
!   your Initialise() routine so that it ends:
!
!       [ Initialise;
!           ...
!           print "^Your introductory text...^^";
!           Banner(); print (string) LICENSE; return 2;
!       ];
!
! ---------------------------------------------------------------------------- !
!   Implements a LICENSE verb which uses Menus.h (or any compatible menu
!   package) to display a menu of the short (friendly) and long (legal) forms
!   of the Creative Commons Attribution-NoDerivs-NonCommercial 1.0 license.
!   If you omit Menus.h, the basic LICENSE verb is extended to display the
!   material using LICENSE S[HORT], LICENSE L[ONG] and LICENSE I[NFO].
! ---------------------------------------------------------------------------- !

system_file;

#ifndef MENU;
Class   Menu with  select
           "Use LICENSE SHORT for a summary, LICENSE LONG for the full text,
            and LICENSE INFO for background information on Creative Commons.";
#endif;
#ifndef OPTION;
Class   Option;
#endif;

[ b text; style bold; print (string) text; style roman; ];

Menu    license_menu
  with  short_name [; print "License for ", (string) STORY; rtrue; ];

Option  license_menu_s "Short friendly form" license_menu with description [; print

    "This work is covered by the ",
(b) "Attribution-NoDerivs-NonCommercial 1.0 ",
    "license published by Creative Commons. In outline, those keywords mean:
     ^^",
(b) "Attribution. ",
    "The licensor permits others to copy, distribute, display, and perform the work.
     In return, licensees must give the original author credit.
     ^^",
(b) "No Derivative Works. ",
    "The licensor permits others to copy, distribute, display and perform only
     unaltered copies of the work -- not derivative works based on it.
     ^^",
(b) "Non Commercial. ",
    "The licensor permits others to copy, distribute, display, and perform the work.
     In return, licensees may not use the work for commercial purposes --
     unless they get the licensor's permission.
     ^^
     The full text of the license is available from the menu, or from this URL:
     ^",
(b) "http://creativecommons.org/licenses/by-nd-nc/1.0/legalcode",
    "^";
];

Option  license_menu_l "Long legal form" license_menu with description [; print

    "THE WORK (AS DEFINED BELOW) IS PROVIDED UNDER THE TERMS
     OF THIS CREATIVE COMMONS PUBLIC LICENSE (~CCPL~ OR ~LICENSE~).
     THE WORK IS PROTECTED BY COPYRIGHT AND/OR OTHER APPLICABLE LAW.
     ANY USE OF THE WORK OTHER THAN AS AUTHORIZED UNDER THIS LICENSE
     IS PROHIBITED.
     ^^
     BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED HERE,
     YOU ACCEPT AND AGREE TO BE BOUND BY THE TERMS OF THIS LICENSE.
     THE LICENSOR GRANTS YOU THE RIGHTS CONTAINED HERE IN
     CONSIDERATION OF YOUR ACCEPTANCE OF SUCH TERMS AND CONDITIONS.
     ^^",
(b) "1. DEFINITIONS",
    "^^a. ~",
(b) "Collective Work",
    "~ means a work, such as a periodical issue, anthology or encyclopedia,
     in which the Work in its entirety in unmodified form, along with a number
     of other contributions, constituting separate and independent works
     in themselves, are assembled into a collective whole.
     A work that constitutes a Collective Work will not be considered a
     Derivative Work (as defined below) for the purposes of this License.
     ^^b. ~",
(b) "Derivative Work",
    "~ means a work based upon the Work or upon the Work and other pre-existing
     works, such as a translation, musical arrangement, dramatization,
     fictionalization, motion picture version, sound recording, art reproduction,
     abridgment, condensation, or any other form in which the Work may be recast,
     transformed, or adapted, except that a work that constitutes a Collective
     Work will not be considered a  Derivative Work for the purpose
     of this License.
     ^^c. ~",
(b) "Licensor",
    "~ means the individual or entity that offers the Work under the terms
     of this License.
     ^^d. ~",
(b) "Original Author",
    "~ means the individual or entity who created the Work.
     ^^e. ~",
(b) "Work",
    "~ means the copyrightable work of authorship offered under the terms
     of this License.
     ^^f. ~",
(b) "You",
    "~ means an individual or entity exercising rights under this License
     who has not previously violated the terms of this License with respect
     to the Work, or who has received express permission from the Licensor
     to exercise rights under this License despite a previous violation.
     ^^",
(b) "2. FAIR USE RIGHTS",
    "^^
     Nothing in this license is intended to reduce, limit, or restrict any rights
     arising from fair use, first sale or other limitations on the exclusive
     rights of the copyright owner under copyright law or other applicable laws.
     ^^",
(b) "3. LICENSE GRANT",
    "^^
     Subject to the terms and conditions of this License, Licensor hereby grants
     You a worldwide, royalty-free, non-exclusive, perpetual (for the duration of
     the applicable copyright) license to exercise the rights in the Work
     as stated below:
     ^^",
(b) "a. ",
    "to reproduce the Work, to incorporate the Work into one or more Collective
     Works, and to reproduce the Work as incorporated in the Collective Works;
     ^^",
(b) "b. ",
    "to distribute copies or phonorecords of, display publicly, perform publicly,
     and perform publicly by means of a digital audio transmission the Work
     including as incorporated in Collective Works;
     ^^
     The above rights may be exercised in all media and formats whether now known
     or hereafter devised.
     The above rights include the right to make such modifications as are technically
     necessary to exercise the rights in other media and formats.
     All rights not expressly granted by Licensor are hereby reserved.
     ^^",
(b) "4. RESTRICTIONS",
    "^^
     The license granted in Section 3 above is expressly made subject to and
     limited by the following restrictions:
     ^^",
(b) "a. ",
    "You may distribute, publicly display, publicly perform, or publicly digitally
     perform the Work only under the terms of this License, and You must include
     a copy of, or the Uniform Resource Identifier for, this License with every
     copy or phonorecord of the Work You distribute, publicly display, publicly
     perform, or publicly digitally perform.
     You may not offer or impose any terms on the Work that alter or restrict
     the terms of this License or the recipients' exercise of the rights granted
     hereunder.
     You may not sublicense the Work.
     You must keep intact all notices that refer to this License and to the
     disclaimer of warranties.
     You may not distribute, publicly display, publicly perform, or publicly
     digitally perform the Work
     with any technological measures that control access or use of the Work
     in a manner inconsistent with the terms of this License Agreement.
     The above applies to the Work as incorporated in a Collective Work,
     but this does not require the Collective Work apart from the Work itself
     to be made subject to the terms of this License.
     If You create a Collective Work, upon notice from any Licensor You must,
     to the extent practicable, remove from the Collective Work any reference
     to such Licensor or the Original Author, as requested.
     ^^",
(b) "b. ",
    "You may not exercise any of the rights granted to You in Section 3 above
     in any manner that is primarily intended for or directed toward commercial
     advantage or private monetary compensation.
     The exchange of the Work for other copyrighted works by means of digital
     file-sharing or otherwise shall not be considered to be intended for or
     directed toward commercial advantage or private monetary compensation,
     provided there is no payment of any monetary compensation in connection
     with the exchange of copyrighted works.
     ^^",
(b) "c. ",
    "If you distribute, publicly display, publicly perform, or publicly digitally
     perform the Work or any Collective Works, You must keep intact all copyright
     notices for the Work and give the Original Author credit reasonable to the
     medium or means You are utilizing by conveying the name (or pseudonym
     if applicable) of the Original Author if supplied;
     the title of the Work if supplied.
     Such credit may be implemented in any reasonable manner; provided, however,
     that in the case of a Collective Work, at a minimum such credit will appear
     where any other comparable authorship credit appears and in a manner at least
     as prominent as such other comparable authorship credit.
     ^^",
(b) "5. REPRESENTATIONS, WARRANTIES and DISCLAIMER",
    "^^",
(b) "a. ",
    "By offering the Work for public release under this License,
     Licensor represents and warrants that,
     to the best of Licensor's knowledge after reasonable inquiry:
     ^",
(b) "(i) ",
    "Licensor has secured all rights in the Work necessary to grant the license
     rights hereunder and to permit the lawful exercise of the rights granted
     hereunder without You having any obligation to pay any royalties,
     compulsory license fees, residuals or any other payments;
     ^",
(b) "(ii) ",
    "The Work does not infringe the copyright, trademark, publicity rights,
     common law rights or any other right of any third party or constitute
     defamation, invasion of privacy or other tortious injury to any third party.
     ^^",
(b) "b. ",
    "EXCEPT AS EXPRESSLY STATED IN THIS LICENSE OR OTHERWISE AGREED IN WRITING
     OR REQUIRED BY APPLICABLE LAW, THE WORK IS LICENSED ON AN ~AS IS~ BASIS,
     WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
     LIMITATION, ANY WARRANTIES REGARDING THE CONTENTS OR ACCURACY OF THE WORK.
     ^^",
(b) "6. LIMITATION on LIABILITY",
    "^^
     EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, AND EXCEPT FOR DAMAGES
     ARISING FROM LIABILITY TO A THIRD PARTY RESULTING FROM BREACH OF THE
     WARRANTIES IN SECTION 5, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU ON
     ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR
     EXEMPLARY DAMAGES ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK,
     EVEN IF LICENSOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
     ^^",
(b) "7. TERMINATION",
    "^^",
(b) "a. ",
    "This License and the rights granted hereunder will terminate automatically
     upon any breach by You of the terms of this License.
     Individuals or entities who have received Collective Works from You under
     this License, however, will not have their licenses terminated
     provided such individuals or entities remain in full compliance with those
     licenses.
     Sections 1, 2, 5, 6, 7, and 8 will survive any termination of this License.
     ^^",
(b) "b. ",
    "Subject to the above terms and conditions, the license granted here is
     perpetual (for the duration of the applicable copyright in the Work).
     Notwithstanding the above, Licensor reserves the right to release the Work
     under different license terms or to stop distributing the Work at any time;
     provided, however that any such election will not serve to withdraw this
     License (or any other license that has been, or is required to be,
     granted under the terms of this License), and this License will continue in
     full force and effect unless terminated as stated above.
     ^^",
(b) "8. MISCELLANEOUS",
    "^^",
(b) "a. ",
    "Each time You distribute or publicly digitally perform the Work or a
     Collective Work, the Licensor offers to the recipient a license to the Work on
     the same terms and conditions as the license granted to You under this License.
     ^^",
(b) "b. ",
    "If any provision of this License is invalid or unenforceable under applicable
     law, it shall not affect the validity or enforceability of the remainder of
     the terms of this License, and without further action by the parties to this
     agreement, such provision shall be reformed to the minimum extent necessary
     to make such provision valid and enforceable.
     ^^",
(b) "c. ",
    "No term or provision of this License shall be deemed waived and no breach
     consented to unless such waiver or consent shall be in writing and signed by
     the party to be charged with such waiver or consent.
     ^^",
(b) "d. ",
    "This License constitutes the entire agreement between the parties with respect
     to the Work licensed here.
     There are no understandings, agreements or representations with respect
     to the Work not specified here.
     Licensor shall not be bound by any additional provisions that may appear
     in any communication from You.
     This License may not be modified without the mutual written agreement of
     the Licensor and You.
     ^";
];

Option  license_menu_i "About Creative Commons" license_menu with description [; print

(b) "Creative Commons ",
    "wants to help define the spectrum of possibilities between full copyright --
     all rights reserved -- and the public domain -- no rights reserved.
     Their licenses help authors retain their copyright while allowing certain
     uses of their work.
     They help authors offer their creative work with some rights reserved.
     ^^
     When you create a work, it's automatically protected by full copyright --
     whether you file for protection or not; whether you display the copyright
     symbol (c) or not.
     This is fine for people who want control over every last use of their work,
     but what about those people who want to share their work on certain terms?
     ^^
     Creative Commons licenses are designed for those folks -- those who
     understand that innovation and new ideas come from building off existing ones.
     ^^
     Every Creative Commons license allows the world to distribute, display, copy
     and webcast an author's work -- provided they abide by certain conditions
     of the author's choice.
     ^^
     For more information, visit the website at ",
(b) "http://creativecommons.org/ ",
    "or send a letter to Creative Commons, 559 Nathan Abbott Way,
     Stanford, California 94305, USA.
     ^";
];

Constant LICENSE
    "This work is licensed under the Creative Commons
     Attribution-NoDerivs-NonCommercial License.
     Type LICENSE to find out more about the terms of the license.^";

[ LicenseSub;      license_menu.select(); ];
[ LicenseShortSub; license_menu_s.description(); ];
[ LicenseLongSub;  license_menu_l.description(); ];
[ LicenseInfoSub;  license_menu_i.description(); ];

Verb 'license' 'licence' 'lic'
    *             -> License
    * 's'/'short' -> LicenseShort
    * 'l'/'long'  -> LicenseLong
    * 'i'/'info'  -> LicenseInfo;

! ---------------------------------------------------------------------------- !
