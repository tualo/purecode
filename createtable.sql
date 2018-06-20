CREATE TABLE `sv_images_extended` (
  `id` varchar(36) NOT NULL,
  `createdate` datetime DEFAULT NULL,
  `server` varchar(100) DEFAULT NULL,
  `filename` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uidx_sv_images_extended` (`server`,`filename`)
);

CREATE TABLE `sv_images_extended_codes` (
  `id` varchar(36) NOT NULL,
  `code` varchar(200) NOT NULL,
  PRIMARY KEY (`id`,`code`),
  CONSTRAINT `fk_sv_images_extended_codes_sv_images_extended` FOREIGN KEY (`id`) REFERENCES `sv_images_extended` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
);

PCNA 2743640 000146784990969962090035C7260308412240100014779
PCNA 2743840 000278007990899962090035C91B0F28419960100027939

20142001 3900320 0004563410774503