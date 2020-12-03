-- MySQL dump 10.10
--
-- Host: localhost    Database: jix
-- ------------------------------------------------------
-- Server version	5.0.22-Debian_0ubuntu6.06.2-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Dumping data for table `groups_services`
--


/*!40000 ALTER TABLE `groups_services` DISABLE KEYS */;
LOCK TABLES `groups_services` WRITE;
INSERT INTO `groups_services` VALUES (1,'gateway','A gateway to the Internet',NULL,NULL),(2,'samba','Protected file storage',NULL,NULL);
UNLOCK TABLES;
/*!40000 ALTER TABLE `groups_services` ENABLE KEYS */;

--
-- Dumping data for table `groups_services_map`
--


/*!40000 ALTER TABLE `groups_services_map` DISABLE KEYS */;
LOCK TABLES `groups_services_map` WRITE;
INSERT INTO `groups_services_map` VALUES ('ekludge',1,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `groups_services_map` ENABLE KEYS */;


--
-- Dumping data for table `users_services`
--


/*!40000 ALTER TABLE `users_services` DISABLE KEYS */;
LOCK TABLES `users_services` WRITE;
INSERT INTO `users_services` VALUES (1,'pipeline','Connect to the Internet thru your own static ip via a tunnel',NULL,NULL),(2,'webadmin','Manage your services yourself',NULL,NULL),(3,'mppe','Secure your pipeline with Microsoft Point-To-Point Encryption',NULL,NULL);
UNLOCK TABLES;
/*!40000 ALTER TABLE `users_services` ENABLE KEYS */;

--
-- Dumping data for table `users_services_map`
--


/*!40000 ALTER TABLE `users_services_map` DISABLE KEYS */;
LOCK TABLES `users_services_map` WRITE;
INSERT INTO `users_services_map` VALUES (1,1,1),(1,2,1),(2,1,1),(2,2,1),(2,3,1),(3,1,1),(3,2,1),(3,3,1),(4,1,0),(4,2,0),(4,3,0),(5,1,1),(5,3,1),(6,1,1),(6,3,1),(7,1,0),(8,1,1),(9,2,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `users_services_map` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

