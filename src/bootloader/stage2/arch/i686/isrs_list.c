#include "idt.h"

#define DEFINE_ISR(n) __attribute__((cdecl)) void i686_isr##n(void);

DEFINE_ISR(0)
DEFINE_ISR(1)
DEFINE_ISR(2)
DEFINE_ISR(3)
DEFINE_ISR(4)
DEFINE_ISR(5)
DEFINE_ISR(6)
DEFINE_ISR(7)
DEFINE_ISR(8)
DEFINE_ISR(9)
DEFINE_ISR(10)
DEFINE_ISR(11)
DEFINE_ISR(12)
DEFINE_ISR(13)
DEFINE_ISR(14)
DEFINE_ISR(15)
DEFINE_ISR(16)
DEFINE_ISR(17)
DEFINE_ISR(18)
DEFINE_ISR(19)
DEFINE_ISR(20)
DEFINE_ISR(21)
DEFINE_ISR(22)
DEFINE_ISR(23)
DEFINE_ISR(24)
DEFINE_ISR(25)
DEFINE_ISR(26)
DEFINE_ISR(27)
DEFINE_ISR(28)
DEFINE_ISR(29)
DEFINE_ISR(30)
DEFINE_ISR(31)
DEFINE_ISR(32)
DEFINE_ISR(33)
DEFINE_ISR(34)
DEFINE_ISR(35)
DEFINE_ISR(36)
DEFINE_ISR(37)
DEFINE_ISR(38)
DEFINE_ISR(39)
DEFINE_ISR(40)
DEFINE_ISR(41)
DEFINE_ISR(42)
DEFINE_ISR(43)
DEFINE_ISR(44)
DEFINE_ISR(45)
DEFINE_ISR(46)
DEFINE_ISR(47)
DEFINE_ISR(48)
DEFINE_ISR(49)
DEFINE_ISR(50)
DEFINE_ISR(51)
DEFINE_ISR(52)
DEFINE_ISR(53)
DEFINE_ISR(54)
DEFINE_ISR(55)
DEFINE_ISR(56)
DEFINE_ISR(57)
DEFINE_ISR(58)
DEFINE_ISR(59)
DEFINE_ISR(60)
DEFINE_ISR(61)
DEFINE_ISR(62)
DEFINE_ISR(63)
DEFINE_ISR(64)
DEFINE_ISR(65)
DEFINE_ISR(66)
DEFINE_ISR(67)
DEFINE_ISR(68)
DEFINE_ISR(69)
DEFINE_ISR(70)
DEFINE_ISR(71)
DEFINE_ISR(72)
DEFINE_ISR(73)
DEFINE_ISR(74)
DEFINE_ISR(75)
DEFINE_ISR(76)
DEFINE_ISR(77)
DEFINE_ISR(78)
DEFINE_ISR(79)
DEFINE_ISR(80)
DEFINE_ISR(81)
DEFINE_ISR(82)
DEFINE_ISR(83)
DEFINE_ISR(84)
DEFINE_ISR(85)
DEFINE_ISR(86)
DEFINE_ISR(87)
DEFINE_ISR(88)
DEFINE_ISR(89)
DEFINE_ISR(90)
DEFINE_ISR(91)
DEFINE_ISR(92)
DEFINE_ISR(93)
DEFINE_ISR(94)
DEFINE_ISR(95)
DEFINE_ISR(96)
DEFINE_ISR(97)
DEFINE_ISR(98)
DEFINE_ISR(99)
DEFINE_ISR(100)
DEFINE_ISR(101)
DEFINE_ISR(102)
DEFINE_ISR(103)
DEFINE_ISR(104)
DEFINE_ISR(105)
DEFINE_ISR(106)
DEFINE_ISR(107)
DEFINE_ISR(108)
DEFINE_ISR(109)
DEFINE_ISR(110)
DEFINE_ISR(111)
DEFINE_ISR(112)
DEFINE_ISR(113)
DEFINE_ISR(114)
DEFINE_ISR(115)
DEFINE_ISR(116)
DEFINE_ISR(117)
DEFINE_ISR(118)
DEFINE_ISR(119)
DEFINE_ISR(120)
DEFINE_ISR(121)
DEFINE_ISR(122)
DEFINE_ISR(123)
DEFINE_ISR(124)
DEFINE_ISR(125)
DEFINE_ISR(126)
DEFINE_ISR(127)
DEFINE_ISR(128)
DEFINE_ISR(129)
DEFINE_ISR(130)
DEFINE_ISR(131)
DEFINE_ISR(132)
DEFINE_ISR(133)
DEFINE_ISR(134)
DEFINE_ISR(135)
DEFINE_ISR(136)
DEFINE_ISR(137)
DEFINE_ISR(138)
DEFINE_ISR(139)
DEFINE_ISR(140)
DEFINE_ISR(141)
DEFINE_ISR(142)
DEFINE_ISR(143)
DEFINE_ISR(144)
DEFINE_ISR(145)
DEFINE_ISR(146)
DEFINE_ISR(147)
DEFINE_ISR(148)
DEFINE_ISR(149)
DEFINE_ISR(150)
DEFINE_ISR(151)
DEFINE_ISR(152)
DEFINE_ISR(153)
DEFINE_ISR(154)
DEFINE_ISR(155)
DEFINE_ISR(156)
DEFINE_ISR(157)
DEFINE_ISR(158)
DEFINE_ISR(159)
DEFINE_ISR(160)
DEFINE_ISR(161)
DEFINE_ISR(162)
DEFINE_ISR(163)
DEFINE_ISR(164)
DEFINE_ISR(165)
DEFINE_ISR(166)
DEFINE_ISR(167)
DEFINE_ISR(168)
DEFINE_ISR(169)
DEFINE_ISR(170)
DEFINE_ISR(171)
DEFINE_ISR(172)
DEFINE_ISR(173)
DEFINE_ISR(174)
DEFINE_ISR(175)
DEFINE_ISR(176)
DEFINE_ISR(177)
DEFINE_ISR(178)
DEFINE_ISR(179)
DEFINE_ISR(180)
DEFINE_ISR(181)
DEFINE_ISR(182)
DEFINE_ISR(183)
DEFINE_ISR(184)
DEFINE_ISR(185)
DEFINE_ISR(186)
DEFINE_ISR(187)
DEFINE_ISR(188)
DEFINE_ISR(189)
DEFINE_ISR(190)
DEFINE_ISR(191)
DEFINE_ISR(192)
DEFINE_ISR(193)
DEFINE_ISR(194)
DEFINE_ISR(195)
DEFINE_ISR(196)
DEFINE_ISR(197)
DEFINE_ISR(198)
DEFINE_ISR(199)
DEFINE_ISR(200)
DEFINE_ISR(201)
DEFINE_ISR(202)
DEFINE_ISR(203)
DEFINE_ISR(204)
DEFINE_ISR(205)
DEFINE_ISR(206)
DEFINE_ISR(207)
DEFINE_ISR(208)
DEFINE_ISR(209)
DEFINE_ISR(210)
DEFINE_ISR(211)
DEFINE_ISR(212)
DEFINE_ISR(213)
DEFINE_ISR(214)
DEFINE_ISR(215)
DEFINE_ISR(216)
DEFINE_ISR(217)
DEFINE_ISR(218)
DEFINE_ISR(219)
DEFINE_ISR(220)
DEFINE_ISR(221)
DEFINE_ISR(222)
DEFINE_ISR(223)
DEFINE_ISR(224)
DEFINE_ISR(225)
DEFINE_ISR(226)
DEFINE_ISR(227)
DEFINE_ISR(228)
DEFINE_ISR(229)
DEFINE_ISR(230)
DEFINE_ISR(231)
DEFINE_ISR(232)
DEFINE_ISR(233)
DEFINE_ISR(234)
DEFINE_ISR(235)
DEFINE_ISR(236)
DEFINE_ISR(237)
DEFINE_ISR(238)
DEFINE_ISR(239)
DEFINE_ISR(240)
DEFINE_ISR(241)
DEFINE_ISR(242)
DEFINE_ISR(243)
DEFINE_ISR(244)
DEFINE_ISR(245)
DEFINE_ISR(246)
DEFINE_ISR(247)
DEFINE_ISR(248)
DEFINE_ISR(249)
DEFINE_ISR(250)
DEFINE_ISR(251)
DEFINE_ISR(252)
DEFINE_ISR(253)
DEFINE_ISR(254)
DEFINE_ISR(255)

void i686_isr_init_gates(void)
{
    i686_idt_init_entry(0, i686_isr0, 0x08, 0xE);
    i686_idt_init_entry(1, i686_isr1, 0x08, 0xE);
    i686_idt_init_entry(2, i686_isr2, 0x08, 0xE);
    i686_idt_init_entry(3, i686_isr3, 0x08, 0xE);
    i686_idt_init_entry(4, i686_isr4, 0x08, 0xE);
    i686_idt_init_entry(5, i686_isr5, 0x08, 0xE);
    i686_idt_init_entry(6, i686_isr6, 0x08, 0xE);
    i686_idt_init_entry(7, i686_isr7, 0x08, 0xE);
    i686_idt_init_entry(8, i686_isr8, 0x08, 0xE);
    i686_idt_init_entry(9, i686_isr9, 0x08, 0xE);
    i686_idt_init_entry(10, i686_isr10, 0x08, 0xE);
    i686_idt_init_entry(11, i686_isr11, 0x08, 0xE);
    i686_idt_init_entry(12, i686_isr12, 0x08, 0xE);
    i686_idt_init_entry(13, i686_isr13, 0x08, 0xE);
    i686_idt_init_entry(14, i686_isr14, 0x08, 0xE);
    i686_idt_init_entry(15, i686_isr15, 0x08, 0xE);
    i686_idt_init_entry(16, i686_isr16, 0x08, 0xE);
    i686_idt_init_entry(17, i686_isr17, 0x08, 0xE);
    i686_idt_init_entry(18, i686_isr18, 0x08, 0xE);
    i686_idt_init_entry(19, i686_isr19, 0x08, 0xE);
    i686_idt_init_entry(20, i686_isr20, 0x08, 0xE);
    i686_idt_init_entry(21, i686_isr21, 0x08, 0xE);
    i686_idt_init_entry(22, i686_isr22, 0x08, 0xE);
    i686_idt_init_entry(23, i686_isr23, 0x08, 0xE);
    i686_idt_init_entry(24, i686_isr24, 0x08, 0xE);
    i686_idt_init_entry(25, i686_isr25, 0x08, 0xE);
    i686_idt_init_entry(26, i686_isr26, 0x08, 0xE);
    i686_idt_init_entry(27, i686_isr27, 0x08, 0xE);
    i686_idt_init_entry(28, i686_isr28, 0x08, 0xE);
    i686_idt_init_entry(29, i686_isr29, 0x08, 0xE);
    i686_idt_init_entry(30, i686_isr30, 0x08, 0xE);
    i686_idt_init_entry(31, i686_isr31, 0x08, 0xE);
    i686_idt_init_entry(32, i686_isr32, 0x08, 0xE);
    i686_idt_init_entry(33, i686_isr33, 0x08, 0xE);
    i686_idt_init_entry(34, i686_isr34, 0x08, 0xE);
    i686_idt_init_entry(35, i686_isr35, 0x08, 0xE);
    i686_idt_init_entry(36, i686_isr36, 0x08, 0xE);
    i686_idt_init_entry(37, i686_isr37, 0x08, 0xE);
    i686_idt_init_entry(38, i686_isr38, 0x08, 0xE);
    i686_idt_init_entry(39, i686_isr39, 0x08, 0xE);
    i686_idt_init_entry(40, i686_isr40, 0x08, 0xE);
    i686_idt_init_entry(41, i686_isr41, 0x08, 0xE);
    i686_idt_init_entry(42, i686_isr42, 0x08, 0xE);
    i686_idt_init_entry(43, i686_isr43, 0x08, 0xE);
    i686_idt_init_entry(44, i686_isr44, 0x08, 0xE);
    i686_idt_init_entry(45, i686_isr45, 0x08, 0xE);
    i686_idt_init_entry(46, i686_isr46, 0x08, 0xE);
    i686_idt_init_entry(47, i686_isr47, 0x08, 0xE);
    i686_idt_init_entry(48, i686_isr48, 0x08, 0xE);
    i686_idt_init_entry(49, i686_isr49, 0x08, 0xE);
    i686_idt_init_entry(50, i686_isr50, 0x08, 0xE);
    i686_idt_init_entry(51, i686_isr51, 0x08, 0xE);
    i686_idt_init_entry(52, i686_isr52, 0x08, 0xE);
    i686_idt_init_entry(53, i686_isr53, 0x08, 0xE);
    i686_idt_init_entry(54, i686_isr54, 0x08, 0xE);
    i686_idt_init_entry(55, i686_isr55, 0x08, 0xE);
    i686_idt_init_entry(56, i686_isr56, 0x08, 0xE);
    i686_idt_init_entry(57, i686_isr57, 0x08, 0xE);
    i686_idt_init_entry(58, i686_isr58, 0x08, 0xE);
    i686_idt_init_entry(59, i686_isr59, 0x08, 0xE);
    i686_idt_init_entry(60, i686_isr60, 0x08, 0xE);
    i686_idt_init_entry(61, i686_isr61, 0x08, 0xE);
    i686_idt_init_entry(62, i686_isr62, 0x08, 0xE);
    i686_idt_init_entry(63, i686_isr63, 0x08, 0xE);
    i686_idt_init_entry(64, i686_isr64, 0x08, 0xE);
    i686_idt_init_entry(65, i686_isr65, 0x08, 0xE);
    i686_idt_init_entry(66, i686_isr66, 0x08, 0xE);
    i686_idt_init_entry(67, i686_isr67, 0x08, 0xE);
    i686_idt_init_entry(68, i686_isr68, 0x08, 0xE);
    i686_idt_init_entry(69, i686_isr69, 0x08, 0xE);
    i686_idt_init_entry(70, i686_isr70, 0x08, 0xE);
    i686_idt_init_entry(71, i686_isr71, 0x08, 0xE);
    i686_idt_init_entry(72, i686_isr72, 0x08, 0xE);
    i686_idt_init_entry(73, i686_isr73, 0x08, 0xE);
    i686_idt_init_entry(74, i686_isr74, 0x08, 0xE);
    i686_idt_init_entry(75, i686_isr75, 0x08, 0xE);
    i686_idt_init_entry(76, i686_isr76, 0x08, 0xE);
    i686_idt_init_entry(77, i686_isr77, 0x08, 0xE);
    i686_idt_init_entry(78, i686_isr78, 0x08, 0xE);
    i686_idt_init_entry(79, i686_isr79, 0x08, 0xE);
    i686_idt_init_entry(80, i686_isr80, 0x08, 0xE);
    i686_idt_init_entry(81, i686_isr81, 0x08, 0xE);
    i686_idt_init_entry(82, i686_isr82, 0x08, 0xE);
    i686_idt_init_entry(83, i686_isr83, 0x08, 0xE);
    i686_idt_init_entry(84, i686_isr84, 0x08, 0xE);
    i686_idt_init_entry(85, i686_isr85, 0x08, 0xE);
    i686_idt_init_entry(86, i686_isr86, 0x08, 0xE);
    i686_idt_init_entry(87, i686_isr87, 0x08, 0xE);
    i686_idt_init_entry(88, i686_isr88, 0x08, 0xE);
    i686_idt_init_entry(89, i686_isr89, 0x08, 0xE);
    i686_idt_init_entry(90, i686_isr90, 0x08, 0xE);
    i686_idt_init_entry(91, i686_isr91, 0x08, 0xE);
    i686_idt_init_entry(92, i686_isr92, 0x08, 0xE);
    i686_idt_init_entry(93, i686_isr93, 0x08, 0xE);
    i686_idt_init_entry(94, i686_isr94, 0x08, 0xE);
    i686_idt_init_entry(95, i686_isr95, 0x08, 0xE);
    i686_idt_init_entry(96, i686_isr96, 0x08, 0xE);
    i686_idt_init_entry(97, i686_isr97, 0x08, 0xE);
    i686_idt_init_entry(98, i686_isr98, 0x08, 0xE);
    i686_idt_init_entry(99, i686_isr99, 0x08, 0xE);
    i686_idt_init_entry(100, i686_isr100, 0x08, 0xE);
    i686_idt_init_entry(101, i686_isr101, 0x08, 0xE);
    i686_idt_init_entry(102, i686_isr102, 0x08, 0xE);
    i686_idt_init_entry(103, i686_isr103, 0x08, 0xE);
    i686_idt_init_entry(104, i686_isr104, 0x08, 0xE);
    i686_idt_init_entry(105, i686_isr105, 0x08, 0xE);
    i686_idt_init_entry(106, i686_isr106, 0x08, 0xE);
    i686_idt_init_entry(107, i686_isr107, 0x08, 0xE);
    i686_idt_init_entry(108, i686_isr108, 0x08, 0xE);
    i686_idt_init_entry(109, i686_isr109, 0x08, 0xE);
    i686_idt_init_entry(110, i686_isr110, 0x08, 0xE);
    i686_idt_init_entry(111, i686_isr111, 0x08, 0xE);
    i686_idt_init_entry(112, i686_isr112, 0x08, 0xE);
    i686_idt_init_entry(113, i686_isr113, 0x08, 0xE);
    i686_idt_init_entry(114, i686_isr114, 0x08, 0xE);
    i686_idt_init_entry(115, i686_isr115, 0x08, 0xE);
    i686_idt_init_entry(116, i686_isr116, 0x08, 0xE);
    i686_idt_init_entry(117, i686_isr117, 0x08, 0xE);
    i686_idt_init_entry(118, i686_isr118, 0x08, 0xE);
    i686_idt_init_entry(119, i686_isr119, 0x08, 0xE);
    i686_idt_init_entry(120, i686_isr120, 0x08, 0xE);
    i686_idt_init_entry(121, i686_isr121, 0x08, 0xE);
    i686_idt_init_entry(122, i686_isr122, 0x08, 0xE);
    i686_idt_init_entry(123, i686_isr123, 0x08, 0xE);
    i686_idt_init_entry(124, i686_isr124, 0x08, 0xE);
    i686_idt_init_entry(125, i686_isr125, 0x08, 0xE);
    i686_idt_init_entry(126, i686_isr126, 0x08, 0xE);
    i686_idt_init_entry(127, i686_isr127, 0x08, 0xE);
    i686_idt_init_entry(128, i686_isr128, 0x08, 0xE);
    i686_idt_init_entry(129, i686_isr129, 0x08, 0xE);
    i686_idt_init_entry(130, i686_isr130, 0x08, 0xE);
    i686_idt_init_entry(131, i686_isr131, 0x08, 0xE);
    i686_idt_init_entry(132, i686_isr132, 0x08, 0xE);
    i686_idt_init_entry(133, i686_isr133, 0x08, 0xE);
    i686_idt_init_entry(134, i686_isr134, 0x08, 0xE);
    i686_idt_init_entry(135, i686_isr135, 0x08, 0xE);
    i686_idt_init_entry(136, i686_isr136, 0x08, 0xE);
    i686_idt_init_entry(137, i686_isr137, 0x08, 0xE);
    i686_idt_init_entry(138, i686_isr138, 0x08, 0xE);
    i686_idt_init_entry(139, i686_isr139, 0x08, 0xE);
    i686_idt_init_entry(140, i686_isr140, 0x08, 0xE);
    i686_idt_init_entry(141, i686_isr141, 0x08, 0xE);
    i686_idt_init_entry(142, i686_isr142, 0x08, 0xE);
    i686_idt_init_entry(143, i686_isr143, 0x08, 0xE);
    i686_idt_init_entry(144, i686_isr144, 0x08, 0xE);
    i686_idt_init_entry(145, i686_isr145, 0x08, 0xE);
    i686_idt_init_entry(146, i686_isr146, 0x08, 0xE);
    i686_idt_init_entry(147, i686_isr147, 0x08, 0xE);
    i686_idt_init_entry(148, i686_isr148, 0x08, 0xE);
    i686_idt_init_entry(149, i686_isr149, 0x08, 0xE);
    i686_idt_init_entry(150, i686_isr150, 0x08, 0xE);
    i686_idt_init_entry(151, i686_isr151, 0x08, 0xE);
    i686_idt_init_entry(152, i686_isr152, 0x08, 0xE);
    i686_idt_init_entry(153, i686_isr153, 0x08, 0xE);
    i686_idt_init_entry(154, i686_isr154, 0x08, 0xE);
    i686_idt_init_entry(155, i686_isr155, 0x08, 0xE);
    i686_idt_init_entry(156, i686_isr156, 0x08, 0xE);
    i686_idt_init_entry(157, i686_isr157, 0x08, 0xE);
    i686_idt_init_entry(158, i686_isr158, 0x08, 0xE);
    i686_idt_init_entry(159, i686_isr159, 0x08, 0xE);
    i686_idt_init_entry(160, i686_isr160, 0x08, 0xE);
    i686_idt_init_entry(161, i686_isr161, 0x08, 0xE);
    i686_idt_init_entry(162, i686_isr162, 0x08, 0xE);
    i686_idt_init_entry(163, i686_isr163, 0x08, 0xE);
    i686_idt_init_entry(164, i686_isr164, 0x08, 0xE);
    i686_idt_init_entry(165, i686_isr165, 0x08, 0xE);
    i686_idt_init_entry(166, i686_isr166, 0x08, 0xE);
    i686_idt_init_entry(167, i686_isr167, 0x08, 0xE);
    i686_idt_init_entry(168, i686_isr168, 0x08, 0xE);
    i686_idt_init_entry(169, i686_isr169, 0x08, 0xE);
    i686_idt_init_entry(170, i686_isr170, 0x08, 0xE);
    i686_idt_init_entry(171, i686_isr171, 0x08, 0xE);
    i686_idt_init_entry(172, i686_isr172, 0x08, 0xE);
    i686_idt_init_entry(173, i686_isr173, 0x08, 0xE);
    i686_idt_init_entry(174, i686_isr174, 0x08, 0xE);
    i686_idt_init_entry(175, i686_isr175, 0x08, 0xE);
    i686_idt_init_entry(176, i686_isr176, 0x08, 0xE);
    i686_idt_init_entry(177, i686_isr177, 0x08, 0xE);
    i686_idt_init_entry(178, i686_isr178, 0x08, 0xE);
    i686_idt_init_entry(179, i686_isr179, 0x08, 0xE);
    i686_idt_init_entry(180, i686_isr180, 0x08, 0xE);
    i686_idt_init_entry(181, i686_isr181, 0x08, 0xE);
    i686_idt_init_entry(182, i686_isr182, 0x08, 0xE);
    i686_idt_init_entry(183, i686_isr183, 0x08, 0xE);
    i686_idt_init_entry(184, i686_isr184, 0x08, 0xE);
    i686_idt_init_entry(185, i686_isr185, 0x08, 0xE);
    i686_idt_init_entry(186, i686_isr186, 0x08, 0xE);
    i686_idt_init_entry(187, i686_isr187, 0x08, 0xE);
    i686_idt_init_entry(188, i686_isr188, 0x08, 0xE);
    i686_idt_init_entry(189, i686_isr189, 0x08, 0xE);
    i686_idt_init_entry(190, i686_isr190, 0x08, 0xE);
    i686_idt_init_entry(191, i686_isr191, 0x08, 0xE);
    i686_idt_init_entry(192, i686_isr192, 0x08, 0xE);
    i686_idt_init_entry(193, i686_isr193, 0x08, 0xE);
    i686_idt_init_entry(194, i686_isr194, 0x08, 0xE);
    i686_idt_init_entry(195, i686_isr195, 0x08, 0xE);
    i686_idt_init_entry(196, i686_isr196, 0x08, 0xE);
    i686_idt_init_entry(197, i686_isr197, 0x08, 0xE);
    i686_idt_init_entry(198, i686_isr198, 0x08, 0xE);
    i686_idt_init_entry(199, i686_isr199, 0x08, 0xE);
    i686_idt_init_entry(200, i686_isr200, 0x08, 0xE);
    i686_idt_init_entry(201, i686_isr201, 0x08, 0xE);
    i686_idt_init_entry(202, i686_isr202, 0x08, 0xE);
    i686_idt_init_entry(203, i686_isr203, 0x08, 0xE);
    i686_idt_init_entry(204, i686_isr204, 0x08, 0xE);
    i686_idt_init_entry(205, i686_isr205, 0x08, 0xE);
    i686_idt_init_entry(206, i686_isr206, 0x08, 0xE);
    i686_idt_init_entry(207, i686_isr207, 0x08, 0xE);
    i686_idt_init_entry(208, i686_isr208, 0x08, 0xE);
    i686_idt_init_entry(209, i686_isr209, 0x08, 0xE);
    i686_idt_init_entry(210, i686_isr210, 0x08, 0xE);
    i686_idt_init_entry(211, i686_isr211, 0x08, 0xE);
    i686_idt_init_entry(212, i686_isr212, 0x08, 0xE);
    i686_idt_init_entry(213, i686_isr213, 0x08, 0xE);
    i686_idt_init_entry(214, i686_isr214, 0x08, 0xE);
    i686_idt_init_entry(215, i686_isr215, 0x08, 0xE);
    i686_idt_init_entry(216, i686_isr216, 0x08, 0xE);
    i686_idt_init_entry(217, i686_isr217, 0x08, 0xE);
    i686_idt_init_entry(218, i686_isr218, 0x08, 0xE);
    i686_idt_init_entry(219, i686_isr219, 0x08, 0xE);
    i686_idt_init_entry(220, i686_isr220, 0x08, 0xE);
    i686_idt_init_entry(221, i686_isr221, 0x08, 0xE);
    i686_idt_init_entry(222, i686_isr222, 0x08, 0xE);
    i686_idt_init_entry(223, i686_isr223, 0x08, 0xE);
    i686_idt_init_entry(224, i686_isr224, 0x08, 0xE);
    i686_idt_init_entry(225, i686_isr225, 0x08, 0xE);
    i686_idt_init_entry(226, i686_isr226, 0x08, 0xE);
    i686_idt_init_entry(227, i686_isr227, 0x08, 0xE);
    i686_idt_init_entry(228, i686_isr228, 0x08, 0xE);
    i686_idt_init_entry(229, i686_isr229, 0x08, 0xE);
    i686_idt_init_entry(230, i686_isr230, 0x08, 0xE);
    i686_idt_init_entry(231, i686_isr231, 0x08, 0xE);
    i686_idt_init_entry(232, i686_isr232, 0x08, 0xE);
    i686_idt_init_entry(233, i686_isr233, 0x08, 0xE);
    i686_idt_init_entry(234, i686_isr234, 0x08, 0xE);
    i686_idt_init_entry(235, i686_isr235, 0x08, 0xE);
    i686_idt_init_entry(236, i686_isr236, 0x08, 0xE);
    i686_idt_init_entry(237, i686_isr237, 0x08, 0xE);
    i686_idt_init_entry(238, i686_isr238, 0x08, 0xE);
    i686_idt_init_entry(239, i686_isr239, 0x08, 0xE);
    i686_idt_init_entry(240, i686_isr240, 0x08, 0xE);
    i686_idt_init_entry(241, i686_isr241, 0x08, 0xE);
    i686_idt_init_entry(242, i686_isr242, 0x08, 0xE);
    i686_idt_init_entry(243, i686_isr243, 0x08, 0xE);
    i686_idt_init_entry(244, i686_isr244, 0x08, 0xE);
    i686_idt_init_entry(245, i686_isr245, 0x08, 0xE);
    i686_idt_init_entry(246, i686_isr246, 0x08, 0xE);
    i686_idt_init_entry(247, i686_isr247, 0x08, 0xE);
    i686_idt_init_entry(248, i686_isr248, 0x08, 0xE);
    i686_idt_init_entry(249, i686_isr249, 0x08, 0xE);
    i686_idt_init_entry(250, i686_isr250, 0x08, 0xE);
    i686_idt_init_entry(251, i686_isr251, 0x08, 0xE);
    i686_idt_init_entry(252, i686_isr252, 0x08, 0xE);
    i686_idt_init_entry(253, i686_isr253, 0x08, 0xE);
    i686_idt_init_entry(254, i686_isr254, 0x08, 0xE);
    i686_idt_init_entry(255, i686_isr255, 0x08, 0xE);
}