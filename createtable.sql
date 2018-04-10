  create table sv_images_extended(
    id varchar(36) primary key,
    createdate datetime,
    server varchar(100),
    filename varchar(150)
  );

  create table sv_images_extended_codes(
    id varchar(36),
    code varchar(200),
    primary key(id,code),

    CONSTRAINT `fk_sv_images_extended_codes_sv_images_extended` FOREIGN KEY (`id`) REFERENCES `sv_images_extended` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
  );