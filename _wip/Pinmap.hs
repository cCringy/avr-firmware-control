module Pinmap where

import System.IO
import Text.Read (readMaybe)
import Data.List (intercalate)
import Data.Maybe (mapMaybe)

data Pin = Pin
  { name :: String
  , x    :: Int
  , y    :: Int
  } deriving Show

-- removeSpaces :: String -> String
-- removeSpaces [] = []
-- removeSpaces (c:cs)
--   | c == ' ' = removeSpaces cs
--   | otherwise = c:removeSpaces cs

splitLine :: String->String->[String]
splitLine elim= go ""
  where
    go acc [] = [reverse acc]
    go acc (c:cs)
      | c `elem` elim = reverse acc:go "" cs
      | otherwise     = go (c:acc) cs


parsePin :: String -> Maybe Pin
parsePin s = case filter (not . null) (splitLine ":,()" (filter (/=' ') s)) of
             [name,xStr,yStr] -> do
                px <- readMaybe xStr
                py <- readMaybe yStr
                return (Pin name px py)
             _                -> Nothing


configPath :: String
configPath = "./pins.txt"

pinToJSON::Pin->String
pinToJSON p = "{\"name\" :\""++ name p ++"\",\"x\": "++ show(x p)++", \"y\" : " ++ show (y p)++"}"

pinsToJSON::[Pin]->String
pinsToJSON ps = "[\n" ++ intercalate ",\n" (map pinToJSON ps) ++ "\n]"

main::IO ()
main = do 
          content <- readFile configPath
          let pins = mapMaybe parsePin (lines content)
          writeFile "pinout.json" (pinsToJSON pins)