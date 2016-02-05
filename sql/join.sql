DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS posts;

CREATE TABLE users (
  id SERIAL NOT NULL,
  name VARCHAR(255)
);

CREATE TABLE posts (
  id SERIAL NOT NULL,
  user_id INTEGER,
  name VARCHAR(255) NOT NULL
);

INSERT INTO users (name)
VALUES
  -- 1
  ('Евгений Васильевич Базаров'),
  -- 2
  ('Николай Петрович Кирсанов'),
  -- 3
  ('Анна Сергеевна Одинцова'),
  -- 4
  ('Екатерина Сергеевна Локтева'),
  -- 5
  ('Емельян Константинович Атепин'),
  -- 6
  ('Иван Алексеевич Котляров'),
  -- 7
  ('Николай Алексеевич Листницкий'),
  -- 8
  ('Юрий Тимофеевич Некрасов'),
  -- 9
  ('Ефрем Иванович Штоквиц'),
  -- 10
  ('Адам Платонович Пацевич');

INSERT INTO posts (user_id, name)
VALUES
  (1, 'Знакомство с Apache Spark'),
  (1, 'Так ли безопасен Tox, как его малюют?'),
  (10, 'Atlassian User Group — 26 февраля, Москва'),
  (NULL, 'Нагрузочное тестирование из облака'),
  (NULL, 'Обходим РКН в домашней сети'),
  (NULL, 'Анализ текущей ситуации на российском BIM-рынке в области гражданского строительства');

SELECT * FROM users JOIN posts ON users.id = posts.user_id;
SELECT * FROM users INNER JOIN posts ON users.id = posts.user_id;
SELECT * FROM users LEFT OUTER JOIN posts ON users.id = posts.user_id;
SELECT * FROM users RIGHT OUTER JOIN posts ON users.id = posts.user_id;
SELECT * FROM users FULL OUTER JOIN posts ON users.id = posts.user_id;

DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS posts;
