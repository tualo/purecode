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